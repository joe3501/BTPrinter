#ifndef _DATA_UART_H_
#define _DATA_UART_H_

#include <stdio.h>

void data_uart_init(void);
void data_uart_sendbyte(unsigned char data);
//unsigned char data_uart_readbyte(void);
unsigned char uart_rec_byte(void);

//void ENABLE_DATA_UART(void);
//void DISABLE_DATA_UART(void);

#endif
