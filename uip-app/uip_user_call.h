#ifndef user_call_H_
#define user_call_H_

#include <stdint.h>
#include "uip-conf.h"

typedef struct 
{
	uint8_t state;
	int8_t *textptr;
	uint16_t textlen;
}user_tcp_appstate;

enum communication_code
{
	STATE_CMD		= 0,	//�������״̬ 
	STATE_TX_TEST	= 1,	//�����������ݰ�״̬(�ٶȲ���)  
	STATE_RX_TEST	= 2		//�����������ݰ�״̬(�ٶȲ���)  
};

typedef user_tcp_appstate uip_tcp_appstate_t;

extern int8_t tcp_sever_send_data_buff[UIP_CONF_BUFFER_SIZE];
extern int8_t tcp_sever_receive_data_buff[UIP_CONF_BUFFER_SIZE];

#ifndef UIP_APPCALL
  #define UIP_APPCALL tcp_sever_call
#endif

#define UIP_BUF ((struct uip_eth_hdr *)&uip_buf[0])

extern volatile uint8_t g_tcp_server_state;//�ⲿ����Ӧ��ͨ���ı�˱�־�������շ� �Լ�ȡ������

extern void uip_log(char *m);
extern void tcp_server_init(void);
extern void uip_polling(void);
extern void tcp_sever_call(void);


#endif
