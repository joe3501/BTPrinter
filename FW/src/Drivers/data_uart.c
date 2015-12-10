/**
 * @file data_uart.c
 * @brief 
 *			串口5的驱动，系统提供给外部的串口，开发阶段可以用于调试
 * @version V0.0.1
 * @author kent.zhou
 * @date 2015年11月12日
 * @note
 *
 * @copy
 *
 * 此代码为深圳合杰电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
 * 本公司以外的项目。本司保留一切追究权利。
 *
 * <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
 */
/* Private include -----------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "data_uart.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/**
* @brief 初始化数据串口
*/
void data_uart_init(void)
{
	USART_InitTypeDef						USART_InitStructure;
	GPIO_InitTypeDef						GPIO_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	/* Configure UART5 Tx (PC.12) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* Configure UART5 Rx (PD.02) as input floating */
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* 设置串口参数								*/
	USART_InitStructure.USART_BaudRate		= 115200;
	USART_InitStructure.USART_WordLength	= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits		= USART_StopBits_1;
	USART_InitStructure.USART_Parity		= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode			= USART_Mode_Tx;
	USART_Init(UART5, &USART_InitStructure);

	USART_Cmd(UART5, ENABLE);	
}

/**
 * @brief	发送一个字节
 */
void data_uart_sendbyte(unsigned char data)
{
	USART_SendData(UART5, (unsigned short)data);
}

/**
 * @brief	接收一个字节
 */
unsigned char uart_rec_byte(void)
{
	int	i = 0;
	while((USART_GetFlagStatus(UART5,USART_FLAG_RXNE)== RESET)&&(i<400000))
	{
		i++;
	}
	if (i == 400000) 
	{
		return 0x55;
	}
	return  USART_ReceiveData(UART5) & 0xFF;              /* Read one byte from the receive data register         */
}

/**
* @brief 实现此函数可以利用系统函数printf,方便调试时格式输出调试信息
*/
#if 0
int fputc(int ch, FILE *f)
{
	//ENABLE_DATA_UART();
	/* Write a character to the USART */
	
	USART_SendData(UART5, (u8) ch);
	
	while(USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET)
	{
	}
	//	DISABLE_DATA_UART;
	return ch;        
}
#endif