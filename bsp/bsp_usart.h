#ifndef BSP_USART_H_
#define BSP_USART_H_

#include "stdio.h"
#include "stdint.h"

extern int fputc(int ch, FILE *f);
extern void usart1_config(void);
extern void print_byte(uint8_t ch);
extern void print_str(const int8_t *p);
extern void print_n_byte(const int8_t *p,uint16_t len);
extern void print_int(int32_t num);
extern void print_float(float f);
extern void print_arg(const int8_t *p,int32_t val);

#endif
