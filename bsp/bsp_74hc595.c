#include "bsp_74hc595.h"

#define MSB (0x80)
#define LSB (0x01)

#define OE_H 	(GPIOB->BSRR = GPIO_Pin_12)
#define OE_L	(GPIOB->BRR  = GPIO_Pin_12)
#define STCP_H	(GPIOB->BSRR = GPIO_Pin_13)
#define STCP_L	(GPIOB->BRR  = GPIO_Pin_13)
#define SHCP_H	(GPIOB->BSRR = GPIO_Pin_14)
#define SHCP_L	(GPIOB->BRR  = GPIO_Pin_14)
#define DS_H	(GPIOB->BSRR = GPIO_Pin_15)
#define DS_L	(GPIOB->BRR  = GPIO_Pin_15)

#define _SF_DELAY 


#ifdef _SF_DELAY
static volatile void delay(uint32_t t);
#endif
static void _74hc595_send_char(uint8_t ch);

#ifdef _SF_DELAY
static volatile void delay(uint32_t t)
{
	while(t--);
} 
#endif

void _74hc595_config(uint8_t len)
{
	OE_L;
	STCP_L;
	SHCP_L;
	while(len--)
	{
		DS_L;
		SHCP_L;
#ifdef _SF_DELAY
		delay(100);
#endif
		SHCP_H;
	}
	STCP_H;
	
}

void _74hc595_send_char(uint8_t ch)
{
	volatile uint8_t i=0;
	for(i=0;i<8;i++)
	{
		SHCP_L;
		if(ch&MSB)
		{
			OE_H;
		}
		else
		{
			OE_L;
		}
		SHCP_H;
#ifdef _SF_DELAY
		delay(100);
#endif	
		SHCP_H;
	}
}

void _74hc595_send_n_char(const uint8_t *pdata,uint8_t n)
{
	STCP_L;
	while(n--)
	{
		_74hc595_send_char(*pdata++);
	}
	STCP_H;
}