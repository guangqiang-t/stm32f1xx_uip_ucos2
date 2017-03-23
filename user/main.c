#include "stm32f10x.h" 
#include "stm32f10x_it.h"
#include "bsp_usart.h"
#include "os_app.h"
#include "uip_user_call.h"
#include "bsp.h"
#include "get_arg.h"

#ifdef DEBUG 
	#define MAIN_LOG(x) print_str(x)
#else
	#define MAIN_LOG(x) 
#endif

int main (void)
{
	usart1_config();
	led_init();
	NVIC_Config();
	pwm_config(100);
	sync_pwm(led_w,0);
	sync_pwm(motor,30);
	tcp_server_init();

	MAIN_LOG("system init ok\r\n");
	
	OSInit();
	OSTaskCreate(start_task, (void *)0, (OS_STK *)&Start_Task_STK[Start_STK_Size-1], Start_Task_PRIO);  //创建起始任务
	OSStart();  //ucos启动
	while(1)
	{
		print_str("uC/OS is error!!!\r\n");
	}
	//return 0;
}

