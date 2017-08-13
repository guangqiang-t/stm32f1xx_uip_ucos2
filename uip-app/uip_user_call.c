#include "uip_user_call.h"

#include <string.h>

#include "stm32f10x.h"

#include "bsp_spi.h"
#include "bsp_usart.h"

#include "uip.h"
#include "timer.h"
#include "tapdev.h"

#include "uip-conf.h"
#include "uip_arp.h"
#include "os_app.h"


#ifdef DEBUG 
	#define UIP_CALL_LOG(x) print_str(x)
#else
	#define UIP_CALL_LOG(x) 
#endif

static	uint8_t i;
static timer_typedef periodic_timer, arp_timer;

// send receive data buffer
int8_t tcp_sever_send_data_buff[UIP_CONF_BUFFER_SIZE]="\0";
int8_t tcp_sever_receive_data_buff[UIP_CONF_BUFFER_SIZE]="\0";

//[7]:0,无连接;1,已经连接;
//[6]:0,无数据;1,收到客户端数据
//[5]:0,无数据;1,有数据需要发送
volatile uint8_t g_tcp_server_state=0;


//tcp server fun
static void tcp_server_aborted(void);
static void tcp_server_timedout(void);
static void tcp_server_closed(void);
static void tcp_server_connected(void);
static void tcp_server_acked(void);
static void tcp_server_senddata(void);


void uip_log(char *m)
{
#if(UIP_CONF_LOGGING)
	while(*m)
	{
		while( USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET ){;}
		USART_SendData(USART1, (uint8_t) *m++);
	}
#else
	
#endif
}



void tcp_server_init(void)
{
//	timer_typedef periodic_timer, arp_timer;
	uip_ipaddr_t ipaddr;
    
	spi1_config();
	
	timer_set(&periodic_timer,CLOCK_SECOND/2);  //创建1个0.5秒的定时器 
	timer_set(&arp_timer,CLOCK_SECOND*10);	   	//创建1个10秒的定时器 
	timer_config();
	
	tapdev_init();      
	uip_init();        
		
	uip_ipaddr(ipaddr, 192,168,1,128);    
	uip_sethostaddr(ipaddr);
		
	uip_ipaddr(ipaddr, 192,168,1,1);                
	uip_setdraddr(ipaddr);
		
	uip_ipaddr(ipaddr, 255,255,255,0);            
	uip_setnetmask(ipaddr); 
	UIP_CALL_LOG("ip init ok...\r\n");
	uip_listen(HTONS(8080)); 
	UIP_CALL_LOG("listen 8080 ...\r\n");
    
}

void uip_polling(void)
{
 
	uip_len=tapdev_read();	//从网络设备读取一个IP包,得到数据长度.uip_len在uip.c中定义
	if(uip_len>0) 			//有数据
	{   
		//处理IP数据包(只有校验通过的IP包才会被接收) 
		if(UIP_BUF->type == htons(UIP_ETHTYPE_IP))//是否是IP包? 
		{
			uip_arp_ipin();	//去除以太网头结构，更新ARP表
			uip_input();   	//IP包处理
			
			//当上面的函数执行后，如果需要发送数据，则全局变量 uip_len > 0
			//需要发送的数据在uip_buf, 长度是uip_len  (这是2个全局变量)		    
			if(uip_len>0)//需要回应数据
			{
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}else if (UIP_BUF->type==htons(UIP_ETHTYPE_ARP))//处理arp报文,是否是ARP请求包?
		{
			uip_arp_arpin();
 			//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len(这是2个全局变量)
 			if(uip_len>0)tapdev_send();//需要发送数据,则通过tapdev_send发送	 
		}
	}else if(timer_expired(&periodic_timer))	//0.5秒定时器超时
	{
		timer_reset(&periodic_timer);		//复位0.5秒定时器 
		//轮流处理每个TCP连接, UIP_CONNS缺省是40个  
		for(i=0;i<UIP_CONNS;i++)
		{
			uip_periodic(i);	//处理TCP通信事件  
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
	 		if(uip_len>0)
			{
				//printf("uip_len is :%d\r\n",uip_len);
				//printf("%s\r\n",uip_buf);
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}
#if UIP_UDP	//UIP_UDP 
		//轮流处理每个UDP连接, UIP_UDP_CONNS缺省是10个
		for(i=0;i<UIP_UDP_CONNS;i++)
		{
			uip_udp_periodic(i);	//处理UDP通信事件
	 		//当上面的函数执行后，如果需要发送数据，则全局变量uip_len>0
			//需要发送的数据在uip_buf, 长度是uip_len (这是2个全局变量)
			if(uip_len > 0)
			{
				uip_arp_out();//加以太网头结构，在主动连接时可能要构造ARP请求
				tapdev_send();//发送数据到以太网
			}
		}
#endif 
		//每隔10秒调用1次ARP定时器函数 用于定期ARP处理,ARP表10秒更新一次，旧的条目会被抛弃
		if(timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
}

void tcp_sever_call(void)
{
 	user_tcp_appstate *StatePointer = (user_tcp_appstate *)&uip_conn->appstate;
	//uip_log("enter tcp_app_call\r\n");
	if(uip_aborted())tcp_server_aborted();		//连接终止
 	if(uip_timedout())tcp_server_timedout();	//连接超时   
	if(uip_closed())tcp_server_closed();		//连接关闭	   
 	if(uip_connected())tcp_server_connected();	//连接成功	    
	if(uip_acked())tcp_server_acked();			//发送的数据成功送达 

	//接收到一个新的TCP数据包 
	if (uip_newdata())//收到客户端发过来的数据
	{
		if((g_tcp_server_state&(1<<6))==0)//还未收到数据
		{
			if(uip_len>127)
			{		   
				((uint8_t*)uip_appdata)[127]=0;
			}		    
				strcpy((char*)tcp_sever_receive_data_buff,uip_appdata);			
				g_tcp_server_state|=1<<6;//表示收到客户端数据
				uip_log("new data\r\n");
		}
	}
	else if(g_tcp_server_state&(1<<5))//有数据需要发送
	{
		StatePointer->textptr=tcp_sever_send_data_buff;
		StatePointer->textlen=strlen((const char*)tcp_sever_send_data_buff);
		memset(tcp_sever_send_data_buff,UIP_CONF_BUFFER_SIZE,0);
		g_tcp_server_state&=~(1<<5);//清除标记
	} 


	//当需要重发、新数据到达、数据包送达、连接建立时，通知uip发送数据 
	if(uip_rexmit()||uip_newdata()||uip_acked()||uip_connected()||uip_poll())
	{
		tcp_server_senddata();
	}
}	  

//终止连接				    
void tcp_server_aborted(void)
{
	g_tcp_server_state&=~(1<<7);	//标志没有连接
	uip_log("tcp_server aborted!\r\n");//打印log
}
//连接超时
void tcp_server_timedout(void)
{
	g_tcp_server_state&=~(1<<7);	//标志没有连接
	uip_log("tcp_server timeout!\r\n");//打印log
}
//连接关闭
void tcp_server_closed(void)
{
	g_tcp_server_state&=~(1<<7);	//标志没有连接
	uip_log("tcp_server closed!\r\n");//打印log
}
//连接建立
void tcp_server_connected(void)
{								  
//	user_tcp_appstate *StatePointer = ( user_tcp_appstate *)&uip_conn->appstate;
	//uip_conn结构体有一个"appstate"字段指向应用程序自定义的结构体。
	//声明一个StatePointer指针，是为了便于使用。
 	//不需要再单独为每个uip_conn分配内存，这个已经在uip中分配好了。
	//在uip.c 中 的相关代码如下：
	//		struct uip_conn *uip_conn;
	//		struct uip_conn uip_conns[UIP_CONNS]; //UIP_CONNS缺省=10
	//定义了1个连接的数组，支持同时创建几个连接。
	//uip_conn是一个全局的指针，指向当前的tcp或udp连接。
	g_tcp_server_state|=1<<7;		//标志连接成功
	uip_log("tcp_server connected!\r\n");//打印log
//	StatePointer->state=STATE_CMD; 		//指令状态
//	StatePointer->textptr="Connnect OK!\r\n";
//	StatePointer->textlen=strlen((char *)StatePointer->textptr);
} 
//发送的数据成功送达
void tcp_server_acked(void)
{						    	 
	user_tcp_appstate *StatePointer = (user_tcp_appstate *)&uip_conn->appstate;
	StatePointer->textlen=0;//发送清零
	uip_log("tcp_server acked!\r\n");//表示成功发送		 
}
//发送数据给客户端
void tcp_server_senddata(void)
{
	user_tcp_appstate *StatePointer = (user_tcp_appstate *)&uip_conn->appstate;
//	StatePointer->textptr : 发送的数据包缓冲区指针
//	StatePointer->textlen ：数据包的大小（单位字节）		   
	if(StatePointer->textlen>0)uip_send(StatePointer->textptr, StatePointer->textlen);//发送TCP数据包	 
	strcpy((char *)tcp_sever_send_data_buff,"ERROR_CMD");
}


