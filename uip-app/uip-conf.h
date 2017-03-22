
#ifndef __UIP_CONF_H
#define __UIP_CONF_H

#include <inttypes.h>

// this is uip define  DO NOT edit it!!
typedef uint8_t                   u8_t;
typedef uint16_t                  u16_t;	
typedef unsigned short            uip_stats_t;

/* ���TCP������ */
#define UIP_CONF_MAX_CONNECTIONS  2
/* ���˿ڼ����� */
#define UIP_CONF_MAX_LISTENPORTS  6	  
/* uIP �����С*/
#define UIP_CONF_BUFFER_SIZE      96
/* CPU�ֽ�˳�� */ 
#define UIP_CONF_BYTE_ORDER       UIP_LITTLE_ENDIAN
/* ��־����	 */			
#define UIP_CONF_LOGGING          0
/* UDP֧�ֿ���*/
#define UIP_CONF_UDP              0
/* UDPУ��Ϳ��� */
#define UIP_CONF_UDP_CHECKSUMS    0
/* uIPͳ�ƿ��� */
#define UIP_CONF_STATISTICS       0

// �����û�����ͷ�ļ������޸�
#include "uip_user_call.h"

#endif
