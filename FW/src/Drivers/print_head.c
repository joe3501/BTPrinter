/**
 * @file  print_head.c
 * @brief 蓝牙打印机项目的热敏打印头管理模块
 * @version 1.0
 * @author kent.zhou
 * @date 2015年11月12日
 * @note
*/
#include "Type.h"
#include "print_head.h"
#include "TP.h"
#include "PaperDetect.h"
#include "stm32f10x_lib.h"
#include "TimeBase.h"
#include <assert.h>

/**
* @brief	初始化热敏打印头的SPI接口
* @return     none
* @note                    
*/
static void print_head_SPI_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;

	/* Enable SPI1 and GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure SPI1 pins: SCK and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;			//由于热敏打印头支持的SPI通讯频率最大为10MHZ，所以此处设置为16分频，4.5M比较安全
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);
}


/**
* @brief	初始化热敏打印头相关的控制IO、SPI接口、定时器及AD模块
* @return     none
* @note                    
*/
void print_head_init(void)
{
	TPInit();
	TPPaperSNSInit();
	print_head_SPI_init();
	SetDesity();		//设置打印速度，表现为打印字体的密度
}

/**
* @brief	通过SPI接口发送数据到热敏打印头
* @return     none
* @note                    
*/
void print_head_spi_send_byte(unsigned char c)
{
	volatile short			i = 0;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI1, c);

	/* Wait to receive a byte */
	//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
	//for(i=0; i<10; i++);
	for(i=0; i<5; i++);

	return;
}


