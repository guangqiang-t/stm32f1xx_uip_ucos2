#include "tapdev.h"
#include "uip.h"
#include "uip_arp.h"
#include "bsp_enc28j60.h"

// MAC地址
struct uip_eth_addr   uip_mac;
static uint8_t  ethernet_mac[6] = {0xAC,0x0D,0x25,0x85,0xB3,0xB1};       
/*
********************************************************************************
* 函 数 名: tapdev_init
* 功能说明: 初始化网卡，并赋值MAC地址
* 参    数：无
* 返 回 值: 无
* 使用说明：
* 调用方法：
********************************************************************************
*/
void tapdev_init(void)
{  
	uint8_t i;
    enc28j60_init(ethernet_mac);                    /*初始化enc28j60 赋值MAC地址*/
    for (i = 0; i < 6; i++)
    {
        uip_mac.addr[i] = ethernet_mac[i];
    }
    uip_setethaddr(uip_mac);                  /* 设定uip mac地址*/
}

/*
********************************************************************************
* 函 数 名: tapdev_read
* 功能说明: 读取以太网数据，最大读取数据包为1500字节
* 参    数：无
* 返 回 值: 返回读取数据包大小
* 使用说明：
* 调用方法：
********************************************************************************
*/
uint16_t tapdev_read(void)
{   
    return  enc28j60_packet_receive(uip_buf,1024);
}
/*
********************************************************************************
* 函 数 名: tapdev_send
* 功能说明: 发送以太网数据包
* 参    数：无
* 返 回 值: 无
* 使用说明：操作uIP全局变量 uip_buf和uip_len
* 调用方法：
********************************************************************************
*/
void tapdev_send(void)
{
    enc28j60_packet_send(uip_buf,uip_len);
}
