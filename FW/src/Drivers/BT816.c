/**
* @file BT816.c
* @brief FSC BT816蓝牙模块的驱动
* @version V0.0.1
* @author joe.zhou
* @date 2015年10月10日
* @note   根据模块的响应特点，串口驱动采用 DMA发送和接受 + 串口空闲中断判断响应数据接收完的机制。
* @copy
*
* 此代码为深圳合杰电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*
*/
#include "hw_platform.h"
#if(BT_MODULE == USE_BT816)
#include "BT816.h"
#include "stm32f10x_lib.h"
#include "string.h"
#include <assert.h>
#include "basic_fun.h"
#include "TimeBase.h"
#include "uart.h"

//#define	BT816_DEBUG
#ifdef DEBUG_VER
extern unsigned char debug_buffer[];
extern unsigned int debug_cnt;
#endif

#define BT816_RES_INIT				0x00


//应答类型的响应数据状态
#define BT816_RES_SUCCESS				0x01
#define BT816_RES_INVALID_STATE			0x02
#define BT816_RES_INVALID_SYNTAX		0x03
#define BT816_RES_BUSY					0x04

#define BT816_RES_PAYLOAD				0x05

#define BT816_RES_UNKOWN				0x06

//command format:AT+(Command)[=parameter]<CR><LF>
//response format1:<CR><LF>+(Response)#(code)<CR><LF>
//		   format1:<CR><LF>+(Response)[=payload]<CR><LF>

/**
* @brief BT816S01响应定义  BT816S01->host
*/
typedef struct {
	unsigned short			DataPos;
	unsigned short			DataLength;
	unsigned char			status;
	unsigned char			*DataBuffer;
}TBT816Res;

TBT816Res		BT816_res[MAX_BT_CHANNEL];

static unsigned char	BT816_send_buff[MAX_BT_CHANNEL][32];
unsigned char	BT816_recbuffer[MAX_BT_CHANNEL][BT816_RES_BUFFER_LEN];


static	unsigned char  bt_connect_status;

#define		BT1_CONNECT		(bt_connect_status&(1<<BT1_MODULE))
#define		BT2_CONNECT		(bt_connect_status&(1<<BT2_MODULE))
#define		BT3_CONNECT		(bt_connect_status&(1<<BT3_MODULE))
#define		BT4_CONNECT		(bt_connect_status&(1<<BT4_MODULE))
#define		BT_CONNECT(ch)		(bt_connect_status&(1<<(ch)))



#define	RESET_BT1_DMA()		do{	\
							DMA_Cmd(DMA1_Channel5,DISABLE);\
							DMA1_Channel5->CNDTR = BT816_RES_BUFFER_LEN;\
							DMA_Cmd(DMA1_Channel5,ENABLE);\
							}while(0)

#define	RESET_BT2_DMA()		do{	\
							DMA_Cmd(DMA1_Channel6,DISABLE);\
							DMA1_Channel6->CNDTR = BT816_RES_BUFFER_LEN; \
							DMA_Cmd(DMA1_Channel6,ENABLE);\
							}while(0)

#define	RESET_BT3_DMA()		do{	\
							DMA_Cmd(DMA1_Channel3,DISABLE);\
							DMA1_Channel3->CNDTR = BT816_RES_BUFFER_LEN;\
							DMA_Cmd(DMA1_Channel3,ENABLE);\
							}while(0)

#define	RESET_BT4_DMA()		do{	\
							DMA_Cmd(DMA2_Channel3,DISABLE);\
							DMA2_Channel3->CNDTR = BT816_RES_BUFFER_LEN;\
							DMA_Cmd(DMA2_Channel3,ENABLE);\
							}while(0)
/*
 * @brief: 初始化模块端口
 * @param[in]  unsigned int bt_channel  蓝牙模块的索引
 * @param[in]  unsigned int baudrate	与模块连接的串口波特率
*/
static void BT816_GPIO_config(unsigned int bt_channel,unsigned int baudrate)
{
	GPIO_InitTypeDef				GPIO_InitStructure;
	USART_InitTypeDef				USART_InitStructure;
	DMA_InitTypeDef					DMA_InitStructure;

	//for debug trip
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB , ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);

	//trip1	PB.6  trip2  PB.5  trip3  PB.4  trip4  PB.3
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_3);		//trip4
	GPIO_SetBits(GPIOB, GPIO_Pin_4);		//trip3
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);		//trip2
	GPIO_SetBits(GPIOB, GPIO_Pin_6);		//trip1

#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (bt_channel == BT1_MODULE)
	{
		//串口1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		//B-Reset  PB.9		B-Busy	PB.8
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB, GPIO_Pin_9);
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);

		//B-State  PB.7
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_7;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPD;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		// 使用USART1, PA9,PA10
		/* Configure USART1 Tx (PA.9) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Configure USART1 Rx (PA.10) as input floating				*/
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate		= baudrate;					
		USART_InitStructure.USART_WordLength	= USART_WordLength_8b;
		USART_InitStructure.USART_StopBits		= USART_StopBits_1;
		USART_InitStructure.USART_Parity		= USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode			= USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(USART1, &USART_InitStructure);


		/* DMA clock enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/* fill init structure */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* DMA1 Channel4 (triggered by USART1 Tx event) Config */
		DMA_DeInit(DMA1_Channel4);
		DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)(&USART1->DR);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
		* and DMA_BufferSize are meaningless. So just set them to proper values
		* which could make DMA_Init happy.
		*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_Init(DMA1_Channel4, &DMA_InitStructure);


		//DMA1通道5配置  
		DMA_DeInit(DMA1_Channel5);  
		//外设地址  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART1->DR);  
		//内存地址  
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer[BT1_MODULE];  
		//dma传输方向单向  
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
		//设置DMA在传输时缓冲区的长度  
		DMA_InitStructure.DMA_BufferSize = BT816_RES_BUFFER_LEN;  
		//设置DMA的外设递增模式，一个外设  
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
		//设置DMA的内存递增模式  
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
		//外设数据字长  
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
		//内存数据字长  
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
		//设置DMA的传输模式  
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
		//设置DMA的优先级别  
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
		//设置DMA的2个memory中的变量互相访问  
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
		DMA_Init(DMA1_Channel5,&DMA_InitStructure);  

		//使能通道5 
		DMA_Cmd(DMA1_Channel5,ENABLE);  

		//采用DMA方式接收  
		USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); 

		/* Enable USART1 DMA Tx request */
		USART_DMACmd(USART1, USART_DMAReq_Tx , ENABLE);

		USART_Cmd(USART1, ENABLE);
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (bt_channel == BT2_MODULE)
	{
		//USART2
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		//B-Reset  PC.3		B-Busy	PC.1
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_1 | GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		GPIO_SetBits(GPIOC, GPIO_Pin_3);
		GPIO_ResetBits(GPIOC, GPIO_Pin_1);

		//B-State  PC.2
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPD;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		// 使用USART2, PA2,PA3
		/* Configure USART2 Tx (PA.2) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* Configure USART2 Rx (PA.3) as input floating				*/
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate		= baudrate;					
		USART_InitStructure.USART_WordLength	= USART_WordLength_8b;
		USART_InitStructure.USART_StopBits		= USART_StopBits_1;
		USART_InitStructure.USART_Parity		= USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode			= USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(USART2, &USART_InitStructure);


		/* DMA clock enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/* fill init structure */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* DMA1 Channel7 (triggered by USART2 Tx event) Config */
		DMA_DeInit(DMA1_Channel7);
		DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)(&USART2->DR);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
		* and DMA_BufferSize are meaningless. So just set them to proper values
		* which could make DMA_Init happy.
		*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_Init(DMA1_Channel7, &DMA_InitStructure);


		//DMA1通道6配置  
		DMA_DeInit(DMA1_Channel6);  
		//外设地址  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART2->DR);  
		//内存地址  
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer[BT2_MODULE];  
		//dma传输方向单向  
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
		//设置DMA在传输时缓冲区的长度  
		DMA_InitStructure.DMA_BufferSize = BT816_RES_BUFFER_LEN;  
		//设置DMA的外设递增模式，一个外设  
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
		//设置DMA的内存递增模式  
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
		//外设数据字长  
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
		//内存数据字长  
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
		//设置DMA的传输模式  
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
		//设置DMA的优先级别  
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
		//设置DMA的2个memory中的变量互相访问  
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
		DMA_Init(DMA1_Channel6,&DMA_InitStructure);  

		//使能通道6 
		DMA_Cmd(DMA1_Channel6,ENABLE);  

		//采用DMA方式接收  
		USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); 

		/* Enable USART2 DMA Tx request */
		USART_DMACmd(USART2, USART_DMAReq_Tx , ENABLE);

		USART_Cmd(USART2, ENABLE);
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (bt_channel == BT3_MODULE)
	{
		//USART3
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

		//B-Reset  PE.13		B-Busy	PE.15
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_13 | GPIO_Pin_15;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
		GPIO_Init(GPIOE, &GPIO_InitStructure);
		GPIO_SetBits(GPIOE, GPIO_Pin_13);
		GPIO_ResetBits(GPIOE, GPIO_Pin_15);

		//B-State  PE.14
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_14;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPD;
		GPIO_Init(GPIOE, &GPIO_InitStructure);

		// 使用USART3, PB10,PB11
		/* Configure USART3 Tx (PB.10) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		/* Configure USART3 Rx (PB.11) as input floating				*/
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate		= baudrate;					
		USART_InitStructure.USART_WordLength	= USART_WordLength_8b;
		USART_InitStructure.USART_StopBits		= USART_StopBits_1;
		USART_InitStructure.USART_Parity		= USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode			= USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(USART3, &USART_InitStructure);


		/* DMA clock enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/* fill init structure */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* DMA1 Channel2 (triggered by USART3 Tx event) Config */
		DMA_DeInit(DMA1_Channel2);
		DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)(&USART3->DR);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
		* and DMA_BufferSize are meaningless. So just set them to proper values
		* which could make DMA_Init happy.
		*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_Init(DMA1_Channel2, &DMA_InitStructure);


		//DMA1通道3配置  
		DMA_DeInit(DMA1_Channel3);  
		//外设地址  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);  
		//内存地址  
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer[BT3_MODULE];  
		//dma传输方向单向  
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
		//设置DMA在传输时缓冲区的长度  
		DMA_InitStructure.DMA_BufferSize = BT816_RES_BUFFER_LEN;  
		//设置DMA的外设递增模式，一个外设  
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
		//设置DMA的内存递增模式  
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
		//外设数据字长  
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
		//内存数据字长  
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
		//设置DMA的传输模式  
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
		//设置DMA的优先级别  
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
		//设置DMA的2个memory中的变量互相访问  
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
		DMA_Init(DMA1_Channel3,&DMA_InitStructure);  

		//使能DMA1通道3 
		DMA_Cmd(DMA1_Channel3,ENABLE);  

		//采用DMA方式接收  
		USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); 

		/* Enable USART3 DMA Tx request */
		USART_DMACmd(USART3, USART_DMAReq_Tx , ENABLE);

		USART_Cmd(USART3, ENABLE);
	}
#endif
#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (bt_channel == BT4_MODULE)
	{
		//UART4
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

		//B-Reset  PD.1		B-Busy	PD.0
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
		GPIO_Init(GPIOD, &GPIO_InitStructure);
		GPIO_SetBits(GPIOD, GPIO_Pin_1);
		GPIO_ResetBits(GPIOD, GPIO_Pin_0);

		//B-State  PD.3
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPD;
		GPIO_Init(GPIOD, &GPIO_InitStructure);

		// 使用UART4, PC11,PC10
		/* Configure UART4 Tx (PC.10) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_AF_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		/* Configure UART4 Rx (PC.11) as input floating				*/
		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate		= baudrate;					
		USART_InitStructure.USART_WordLength	= USART_WordLength_8b;
		USART_InitStructure.USART_StopBits		= USART_StopBits_1;
		USART_InitStructure.USART_Parity		= USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode			= USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(UART4, &USART_InitStructure);


		/* DMA clock enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

		/* fill init structure */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* DMA2 Channel5 (triggered by UART4 Tx event) Config */
		DMA_DeInit(DMA2_Channel5);
		DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)(&UART4->DR);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
		* and DMA_BufferSize are meaningless. So just set them to proper values
		* which could make DMA_Init happy.
		*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_Init(DMA2_Channel5, &DMA_InitStructure);


		//DMA2通道3配置  
		DMA_DeInit(DMA2_Channel3);  
		//外设地址  
		DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);  
		//内存地址  
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer[BT4_MODULE];  
		//dma传输方向单向  
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
		//设置DMA在传输时缓冲区的长度  
		DMA_InitStructure.DMA_BufferSize = BT816_RES_BUFFER_LEN;  
		//设置DMA的外设递增模式，一个外设  
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
		//设置DMA的内存递增模式  
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
		//外设数据字长  
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
		//内存数据字长  
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
		//设置DMA的传输模式  
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
		//设置DMA的优先级别  
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
		//设置DMA的2个memory中的变量互相访问  
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
		DMA_Init(DMA2_Channel3,&DMA_InitStructure);  

		//使能DMA2通道3 
		DMA_Cmd(DMA2_Channel3,ENABLE);  

		//采用DMA方式接收  
		USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); 

		/* Enable UART4 DMA Tx request */
		USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);

		USART_Cmd(UART4, ENABLE);
	}
#endif
}

/*
* @brief: 串口中断的初始化
*/
static void BT816_NVIC_config(unsigned int bt_channel)
{
	NVIC_InitTypeDef				NVIC_InitStructure;
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (bt_channel == BT1_MODULE)
	{
		//中断配置  
		USART_ITConfig(USART1,USART_IT_TC,DISABLE);  
		USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);  
		USART_ITConfig(USART1,USART_IT_IDLE,ENABLE);    

		//配置UART4中断  
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQChannel;               //通道设置为串口2中断    
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //中断占先等级1    
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //中断响应优先级0    
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
		NVIC_Init(&NVIC_InitStructure);  

		/* Enable the DMA1 Channel4 Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ITConfig(DMA1_Channel4, DMA_IT_TC | DMA_IT_TE, ENABLE);
		DMA_ClearFlag(DMA1_FLAG_TC4);
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (bt_channel == BT2_MODULE)
	{
		//中断配置  
		USART_ITConfig(USART2,USART_IT_TC,DISABLE);  
		USART_ITConfig(USART2,USART_IT_RXNE,DISABLE);  
		USART_ITConfig(USART2,USART_IT_IDLE,ENABLE);    

		//配置USART2中断  
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQChannel;               //通道设置为串口2中断    
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //中断占先等级1    
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //中断响应优先级0    
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
		NVIC_Init(&NVIC_InitStructure);  

		/* Enable the DMA1 Channel7 Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ITConfig(DMA1_Channel7, DMA_IT_TC | DMA_IT_TE, ENABLE);
		DMA_ClearFlag(DMA1_FLAG_TC7);
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (bt_channel == BT3_MODULE)
	{
		//中断配置  
		USART_ITConfig(USART3,USART_IT_TC,DISABLE);  
		USART_ITConfig(USART3,USART_IT_RXNE,DISABLE);  
		USART_ITConfig(USART3,USART_IT_IDLE,ENABLE);    

		//配置USART3中断  
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;               //通道设置为串口2中断    
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //中断占先等级1    
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //中断响应优先级0    
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
		NVIC_Init(&NVIC_InitStructure);  

		/* Enable the DMA1 Channel2 Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ITConfig(DMA1_Channel2, DMA_IT_TC | DMA_IT_TE, ENABLE);
		DMA_ClearFlag(DMA1_FLAG_TC2);
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (bt_channel == BT4_MODULE)
	{
		//中断配置  
		USART_ITConfig(UART4,USART_IT_TC,DISABLE);  
		USART_ITConfig(UART4,USART_IT_RXNE,DISABLE);  
		USART_ITConfig(UART4,USART_IT_IDLE,ENABLE);    

		//配置UART4中断  
		//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
		NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;               //通道设置为串口2中断    
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;       //中断占先等级1    
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;              //中断响应优先级0    
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
		NVIC_Init(&NVIC_InitStructure);  

		/* Enable the DMA2 Channel5 Interrupt */
		NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQChannel;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);

		DMA_ITConfig(DMA2_Channel5, DMA_IT_TC | DMA_IT_TE, ENABLE);
		DMA_ClearFlag(DMA2_FLAG_TC5);
	}
#endif
}


/**
* @brief  发数据给蓝牙模块
* @param[in] unsigned char *pData 要发送的数据
* @param[in] int length 要发送数据的长度
*/
static void send_data_to_BT(unsigned int bt_channel,const unsigned char *pData, unsigned int length)
{
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (bt_channel == BT1_MODULE)
	{
		//while(length--)
		//{
		//	USART_SendData(USART1, *pData++);
		//	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		//	{
		//	}
		//}
		//while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){};

		/* disable DMA */
		DMA_Cmd(DMA1_Channel4, DISABLE);

		/* set buffer address */
		MEMCPY(BT816_send_buff[BT1_MODULE],pData,length);

		DMA1_Channel4->CMAR = (u32)&BT816_send_buff[BT1_MODULE][0];
		/* set size */
		DMA1_Channel4->CNDTR = length;

		USART_DMACmd(USART1, USART_DMAReq_Tx , ENABLE);
		/* enable DMA */
		DMA_Cmd(DMA1_Channel4, ENABLE);

		while(DMA1_Channel4->CNDTR);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET){};
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (bt_channel == BT2_MODULE)
	{
		//while(length--)
		//{
		//	USART_SendData(USART2, *pData++);
		//	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		//	{
		//	}
		//}
		//while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){};

		/* disable DMA */
		DMA_Cmd(DMA1_Channel7, DISABLE);

		/* set buffer address */
		MEMCPY(BT816_send_buff[BT2_MODULE],pData,length);

		DMA1_Channel7->CMAR = (u32)&BT816_send_buff[BT2_MODULE][0];
		/* set size */
		DMA1_Channel7->CNDTR = length;

		USART_DMACmd(USART2, USART_DMAReq_Tx , ENABLE);
		/* enable DMA */
		DMA_Cmd(DMA1_Channel7, ENABLE);

		while(DMA1_Channel7->CNDTR);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){};
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (bt_channel == BT3_MODULE)
	{
		//while(length--)
		//{
		//	USART_SendData(USART3, *pData++);
		//	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
		//	{
		//	}
		//}
		//while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){};

		/* disable DMA */
		DMA_Cmd(DMA1_Channel2, DISABLE);

		/* set buffer address */
		MEMCPY(BT816_send_buff[BT3_MODULE],pData,length);

		DMA1_Channel2->CMAR = (u32)&BT816_send_buff[BT3_MODULE][0];
		/* set size */
		DMA1_Channel2->CNDTR = length;

		USART_DMACmd(USART3, USART_DMAReq_Tx , ENABLE);
		/* enable DMA */
		DMA_Cmd(DMA1_Channel2, ENABLE);

		while(DMA1_Channel2->CNDTR);
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET){};
	}
#endif
#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (bt_channel == BT4_MODULE)
	{
	//while(length--)
	//{
	//	USART_SendData(UART4, *pData++);
	//	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET)
	//	{
	//	}
	//}
	//while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){};

	/* disable DMA */
	DMA_Cmd(DMA2_Channel5, DISABLE);

	/* set buffer address */
	MEMCPY(BT816_send_buff[BT4_MODULE],pData,length);

	DMA2_Channel5->CMAR = (u32)&BT816_send_buff[BT4_MODULE][0];
	/* set size */
	DMA2_Channel5->CNDTR = length;

	USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);
	/* enable DMA */
	DMA_Cmd(DMA2_Channel5, ENABLE);

	while(DMA2_Channel5->CNDTR);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){};
	}
#endif
}


/*
 * @brief 清空接收蓝牙模块响应数据的buffer
*/
static void BT816_reset_resVar(unsigned int bt_channel)
{
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (bt_channel == BT1_MODULE)
	{
		BT816_res[bt_channel].DataPos = 0;
		BT816_res[bt_channel].DataLength = 0;
		BT816_res[bt_channel].status	 = BT816_RES_INIT;
		bt_connect_status &= ~(1<<BT1_MODULE);
		DMA1_Channel5->CNDTR = BT816_RES_BUFFER_LEN; 
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (bt_channel == BT2_MODULE)
	{
		BT816_res[bt_channel].DataPos = 0;
		BT816_res[bt_channel].DataLength = 0;
		BT816_res[bt_channel].status	 = BT816_RES_INIT;
		bt_connect_status &= ~(1<<BT2_MODULE);
		DMA1_Channel6->CNDTR = BT816_RES_BUFFER_LEN; 
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (bt_channel == BT3_MODULE)
	{
		BT816_res[bt_channel].DataPos = 0;
		BT816_res[bt_channel].DataLength = 0;
		BT816_res[bt_channel].status	 = BT816_RES_INIT;
		bt_connect_status &= ~(1<<BT3_MODULE);
		DMA1_Channel3->CNDTR = BT816_RES_BUFFER_LEN; 
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (bt_channel == BT4_MODULE)
	{
	BT816_res[bt_channel].DataPos = 0;
	BT816_res[bt_channel].DataLength = 0;
	BT816_res[bt_channel].status	 = BT816_RES_INIT;
	bt_connect_status &= ~(1<<BT4_MODULE);
	DMA2_Channel3->CNDTR = BT816_RES_BUFFER_LEN;
	}
#endif
}


/**
* @brief 处理host收到BT816的数据
* @param[in] unsigned char c 读入的字符
* @return 0:success put in buffer
*        -1:fail
*/
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
int BT816_Channel1_RxISRHandler(unsigned char *res, unsigned int res_len)
{	
	int i,len;
	if (BT1_CONNECT)
	{
		//已经处于连接状态，蓝牙模块进入数据透传模式
		set_BT1_BUSY();
		ringbuffer_put(&spp_ringbuf[BT1_MODULE],res,res_len);
#ifdef DEBUG_VER
		//MEMCPY(debug_buffer+debug_cnt,res,res_len);
		//debug_cnt += res_len;
#endif
		DMA1_Channel5->CNDTR = BT816_RES_BUFFER_LEN;
		if (ringbuffer_data_len(&spp_ringbuf[BT1_MODULE]) < RING_BUFF_FULL_TH)
		{
			set_BT1_FREE();
		}	
	}
	else
	{
		if (res_len > 5)
		{
			BT816_res[BT1_MODULE].DataLength = res_len;
			if ((res[0] == 0x0d)&&(res[1] == 0x0a)			\
				&&(res[res_len-2] == 0x0d)&&(res[res_len-1] == 0x0a))
			{
				if (BT816_res[BT1_MODULE].status == BT816_RES_INIT)
				{
					BT816_res[BT1_MODULE].status = BT816_RES_INVALID_STATE;
					for (i = 3; i < res_len-2;i++)
					{

						if ((res[i] == 'o')&&(res[i+1] == 'k'))
						{
							BT816_res[BT1_MODULE].status = BT816_RES_SUCCESS;
							break;
						}

						if (res[i] == '=')
						{
							BT816_res[BT1_MODULE].status = BT816_RES_PAYLOAD;
							break;
						}
					}
				}
			}
			else
			{
				BT816_res[BT1_MODULE].status = BT816_RES_UNKOWN;
			}
		}
	}
	
        
        return 0;
}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
int BT816_Channel2_RxISRHandler(unsigned char *res, unsigned int res_len)
{	
	int i,len;

	if (BT2_CONNECT)
	{
		//已经处于连接状态，蓝牙模块进入数据透传模式
		set_BT2_BUSY();
		ringbuffer_put(&spp_ringbuf[BT2_MODULE],res,res_len);
		DMA1_Channel6->CNDTR = BT816_RES_BUFFER_LEN;
		if (ringbuffer_data_len(&spp_ringbuf[BT2_MODULE]) < RING_BUFF_FULL_TH)
		{
			set_BT2_FREE();
		}
	}
	else
	{
		if (res_len > 5)
		{
			BT816_res[BT2_MODULE].DataLength = res_len;
			if ((res[0] == 0x0d)&&(res[1] == 0x0a)			\
				&&(res[res_len-2] == 0x0d)&&(res[res_len-1] == 0x0a))
			{
				if (BT816_res[BT2_MODULE].status == BT816_RES_INIT)
				{
					BT816_res[BT2_MODULE].status = BT816_RES_INVALID_STATE;
					for (i = 3; i < res_len-2;i++)
					{

						if ((res[i] == 'o')&&(res[i+1] == 'k'))
						{
							BT816_res[BT2_MODULE].status = BT816_RES_SUCCESS;
							break;
						}

						if (res[i] == '=')
						{
							BT816_res[BT2_MODULE].status = BT816_RES_PAYLOAD;
							break;
						}
					}
				}
			}
			else
			{
				BT816_res[BT2_MODULE].status = BT816_RES_UNKOWN;
			}
		}
	}


	return 0;
}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
int BT816_Channel3_RxISRHandler(unsigned char *res, unsigned int res_len)
{	
	int i,len;
	if (BT3_CONNECT)
	{
		//已经处于连接状态，蓝牙模块进入数据透传模式
		set_BT3_BUSY();
		ringbuffer_put(&spp_ringbuf[BT3_MODULE],res,res_len);
		DMA1_Channel3->CNDTR = BT816_RES_BUFFER_LEN;
		if (ringbuffer_data_len(&spp_ringbuf[BT3_MODULE]) < RING_BUFF_FULL_TH)
		{
			set_BT3_FREE();
		}
	}
	else
	{
		if (res_len > 5)
		{
			BT816_res[BT3_MODULE].DataLength = res_len;
			if ((res[0] == 0x0d)&&(res[1] == 0x0a)			\
				&&(res[res_len-2] == 0x0d)&&(res[res_len-1] == 0x0a))
			{
				if (BT816_res[BT3_MODULE].status == BT816_RES_INIT)
				{
					BT816_res[BT3_MODULE].status = BT816_RES_INVALID_STATE;
					for (i = 3; i < res_len-2;i++)
					{

						if ((res[i] == 'o')&&(res[i+1] == 'k'))
						{
							BT816_res[BT3_MODULE].status = BT816_RES_SUCCESS;
							break;
						}

						if (res[i] == '=')
						{
							BT816_res[BT3_MODULE].status = BT816_RES_PAYLOAD;
							break;
						}
					}
				}
			}
			else
			{
				BT816_res[BT3_MODULE].status = BT816_RES_UNKOWN;
			}
		}
	}


	return 0;
}
#endif

#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
int BT816_Channel4_RxISRHandler(unsigned char *res, unsigned int res_len)
{	
	int i,len;

	if (BT4_CONNECT)
	{
		//已经处于连接状态，蓝牙模块进入数据透传模式
		set_BT4_BUSY();
		ringbuffer_put(&spp_ringbuf[BT4_MODULE],res,res_len);
		DMA2_Channel3->CNDTR = BT816_RES_BUFFER_LEN;
		if (ringbuffer_data_len(&spp_ringbuf[BT4_MODULE]) < RING_BUFF_FULL_TH)
		{
			set_BT4_FREE();
		}
	}
	else
	{
		if (res_len > 5)
		{
			BT816_res[BT4_MODULE].DataLength = res_len;
			if ((res[0] == 0x0d)&&(res[1] == 0x0a)			\
				&&(res[res_len-2] == 0x0d)&&(res[res_len-1] == 0x0a))
			{
				if (BT816_res[BT4_MODULE].status == BT816_RES_INIT)
				{
					BT816_res[BT4_MODULE].status = BT816_RES_INVALID_STATE;
					for (i = 3; i < res_len-2;i++)
					{

						if ((res[i] == 'o')&&(res[i+1] == 'k'))
						{
							BT816_res[BT4_MODULE].status = BT816_RES_SUCCESS;
							break;
						}

						if (res[i] == '=')
						{
							BT816_res[BT4_MODULE].status = BT816_RES_PAYLOAD;
							break;
						}
					}
				}
			}
			else
			{
				BT816_res[BT4_MODULE].status = BT816_RES_UNKOWN;
			}
		}
	}


	return 0;
}
#endif
#define EXPECT_RES_FORMAT1_TYPE		1		//设置命令的响应，OK or Error
#define EXPECT_RES_FORMAT2_TYPE		2		//查询命令的响应，payload

/**
* @brief  发命令给蓝牙模块BT816S01,并等待响应结果
* @param[in] unsigned char *pData 要发送的数据
* @param[in] unsigned int	length 要发送数据的长度
* @param[in] unsigned char  type   期待响应数据的命令类型	
*							EXPECT_RES_FORMAT1_TYPE: response format1		
*							EXPECT_RES_FORMAT2_TYPE:response format2
* @return		0: 成功
*				-1: 失败
*				-2: 未知响应，可能是响应解析函数有BUG，需要调试
*				-3：响应超时
* @note	等待一个响应帧的命令
*/
static int BT816_write_cmd(unsigned int bt_channel,const unsigned char *pData, unsigned int length,unsigned char type)
{
	unsigned int	wait_cnt;
	send_data_to_BT(bt_channel,pData, length);
	BT816_reset_resVar(bt_channel);
	wait_cnt = 200;
	while (wait_cnt)
	{
		if (((BT816_res[bt_channel].status == BT816_RES_SUCCESS)&&(type == EXPECT_RES_FORMAT1_TYPE)) || ((BT816_res[bt_channel].status == BT816_RES_PAYLOAD)&&(type == EXPECT_RES_FORMAT2_TYPE)))
		{
			return 0;
		}
		else if(BT816_res[bt_channel].status == BT816_RES_INVALID_STATE)
		{
			return -1;
		}
		//OSTimeDlyHMSM(0,0,0,20);
		delay_ms(20);
                wait_cnt--;
	}

	return -3;
}


//const unsigned char	*query_version_cmd="AT+VER=?";
//const unsigned char	*set_device_name_cmd="AT+DNAME=%s";
static unsigned char	token[10],token_value[15];
/*
 * @brief 蓝牙模块BT816S01的复位
*/
int BT816_Reset(void)
{
	unsigned int	wait_cnt,i,j;
	unsigned char	stat;
	int ret;

#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	//拉低复位信号100ms
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);
	delay_ms(100);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);

	BT816_reset_resVar(BT1_MODULE);
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	//拉低复位信号100ms
	GPIO_ResetBits(GPIOC, GPIO_Pin_3);
	delay_ms(100);
	GPIO_SetBits(GPIOC, GPIO_Pin_3);

	BT816_reset_resVar(BT2_MODULE);
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	//拉低复位信号100ms
	GPIO_ResetBits(GPIOE, GPIO_Pin_13);
	delay_ms(100);
	GPIO_SetBits(GPIOE, GPIO_Pin_13);

	BT816_reset_resVar(BT3_MODULE);
#endif

#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	//拉低复位信号100ms
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);
	delay_ms(100);
	GPIO_SetBits(GPIOD, GPIO_Pin_1);

	BT816_reset_resVar(BT4_MODULE);
#endif
#if 0
	wait_cnt = 20;
	ret = 0;
	while (wait_cnt)
	{
		if (BT816_res[].status == BT816_RES_PAYLOAD)
		{
			USART_Cmd(UART4, DISABLE);
			stat = 0;
			for (i = 0; i < BT816_res.DataLength;i++)
			{
				if (BT816_res.DataBuffer[i] == '+')
				{
					stat = 1;
					j = 0;
					continue;
				}
				else if (BT816_res.DataBuffer[i] == '=')
				{
					stat = 2;
					j = 0;
					continue;
				}
				else if (BT816_res.DataBuffer[i] == 0x0d)
				{
					if (stat == 2)
					{
						if (MEMCMP(token,"BDTP",4)==0)
						{
							if (token_value[0] != '0')
							{
								ret |= 0x01;
							}
						}
#ifdef DEBUG_VER
						else if (MEMCMP(token,"BDVER",5)==0)
						{
							token_value[j]=0;
							printf("BlueTooth Module Ver:%s\r\n",token_value);
						}
						else if (MEMCMP(token,"BDADDR",6)==0)
						{
							token_value[j]=0;
							printf("BlueTooth Module Addr:%s\r\n",token_value);
						}
#endif
						else if (MEMCMP(token,"BDMODE",6)==0)
						{
#ifdef HID_MODE
							if (token_value[0] != '2')
							{
								ret |= 0x02;
							}
#else
							if (token_value[0] != '1')
							{
								ret |= 0x02;
							}
#endif
						}
					}
					stat = 0;
					continue;
				}

				if (stat == 1)
				{
					token[j] = BT816_res.DataBuffer[i];
					j++;
				}
				else if (stat == 2)
				{
					token_value[j] = BT816_res.DataBuffer[i];
					j++;
				}
			} 
			USART_Cmd(UART4, ENABLE);
			return ret;
		}
		//OSTimeDlyHMSM(0,0,0,100);
                wait_cnt--;
                delay_ms(100);
	}

	return -1;
#endif

	delay_ms(1000);
	return 0;
}

/*
 * @brief 查询蓝牙模块BT816的版本号
 * @param[out]  unsigned char *ver_buffer  返回查询到的版本号，如果为空表示查询失败
*/
int BT816_query_version(unsigned int bt_channel,unsigned char *ver_buffer)
{
	unsigned char	buffer[21];
	int		i,ret;

	assert(ver_buffer != 0);
	ver_buffer[0] = 0;
	MEMCPY(buffer,"AT+VER\x0d\x0a",8);
	ret = BT816_write_cmd(bt_channel,(const unsigned char*)buffer,8,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (MEMCMP(&BT816_res[bt_channel].DataBuffer[3],"VER=",4) == 0)
	{
		for (i = 0; i < ((BT816_res[bt_channel].DataLength-9) > 20)?20:(BT816_res[bt_channel].DataLength-9);i++)
		{
			if (BT816_res[bt_channel].DataBuffer[7+i] == 0x0d)
			{
				break;
			}

			ver_buffer[i] = BT816_res[bt_channel].DataBuffer[7+i];
		}
		ver_buffer[i] = 0;
		return 0;
	}

	return -1;
}


/*
 * @brief 查询蓝牙模块的设备名称
 * @param[out]  unsigned char *name  模块名称,字符串
 * @return 0: 查询成功		else：查询失败
 * @note 从手册暂时没有看到支持的名称的最大长度是多少，所以如果设置的名称太长可能会导致缓冲区溢出
 *       在此接口中将设备名称限定为最长支持20个字节
*/
int BT816_query_name(unsigned int bt_channel,unsigned char *name)
{
	unsigned char	buffer[15];
	int		i,ret;

	assert(name != 0);
	name[0] = 0;
	MEMCPY(buffer,"AT+NAME\x0d\x0a",9);

	ret = BT816_write_cmd(bt_channel,(const unsigned char*)buffer,9,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (MEMCMP(&BT816_res[bt_channel].DataBuffer[3],"NAME",4) == 0)
	{
		for (i = 0; i < ((BT816_res[bt_channel].DataLength-10) > 20)?20:(BT816_res[bt_channel].DataLength-10);i++)
		{
			if (BT816_res[bt_channel].DataBuffer[8+i] == 0x0d)
			{
				break;
			}

			name[i] = BT816_res[bt_channel].DataBuffer[8+i];
		}
		name[i] = 0;
		return 0;
	}

	return -1; 
}

/*
 * @brief 查询和设置蓝牙模块的设备名称
 * @param[in]  unsigned char *name  设置的名称,字符串
 * @return 0: 设置成功		else：设置失败
 * @note 从手册暂时没有看到支持的名称的最大长度是多少，所以如果设置的名称太长可能会设置失败
 *       在此接口中将设备名称限定为最长支持22个字节
*/
int BT816_set_name(unsigned int bt_channel,unsigned char *name)
{
	unsigned char	buffer[33];
	int		len;

	assert(name != 0);
	MEMCPY(buffer,"AT+NAME=",8);
	len = STRLEN((char const*)name);
	if (len>22)
	{
		MEMCPY(buffer+8,name,22);
		buffer[30] = 0x0d;
		buffer[31] = 0x0a;
		len = 32;
	}
	else
	{
		MEMCPY(buffer+8,name,len);
		buffer[8+len] = 0x0d;
		buffer[9+len] = 0x0a;
		len += 10;
	}

	return BT816_write_cmd(bt_channel,(const unsigned char*)buffer,len,EXPECT_RES_FORMAT1_TYPE); 
}

/*
 * @brief 设置蓝牙模块的PIN
 * @param[in]  unsigned char *name  设置的PIN,字符串
 * @return 0: 设置成功		else：设置失败
 * @note 从手册暂时没有看到支持的PIN的最大长度是多少，所以如果设置的PIN太长可能会设置失败
 *       在此接口中将设备名称限定为最长支持8个字节
*/
int BT816_set_pin(unsigned int bt_channel,unsigned char *pin)
{
	unsigned char	buffer[33];
	int		len;

	assert(pin != 0);
	MEMCPY(buffer,"AT+PIN=",7);
	len = STRLEN((char const*)pin);
	if (len>8)
	{
		MEMCPY(buffer+7,pin,8);
		buffer[15] = 0x0d;
		buffer[16] = 0x0a;
		len = 17;
	}
	else
	{
		MEMCPY(buffer+7,pin,len);
		buffer[7+len] = 0x0d;
		buffer[8+len] = 0x0a;
		len += 9;
	}

	return BT816_write_cmd(bt_channel,(const unsigned char*)buffer,len,EXPECT_RES_FORMAT1_TYPE); 
}


/*
 * @brief 查询蓝牙模块HID当前的连接状态  	
 * @return <0 ：发送失败	0: unkown 	1: connected    2： disconnect
 * @note 此函数需要被不断轮询才能正确反应各个模块的连接状态
*/
int BT816_connect_status(unsigned int bt_channel)
{
	unsigned int i;
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (bt_channel == BT1_MODULE)
	{
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7))
		{
			for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_7))
			{
				bt_connect_status |= (1<<BT1_MODULE);
				return BT_MODULE_STATUS_CONNECTED;
			}
			else
			{
				bt_connect_status &= ~(1<<BT1_MODULE);
				return BT_MODULE_STATUS_DISCONNECT;
			}
		}
		else
		{
			bt_connect_status &= ~(1<<BT1_MODULE);
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (bt_channel == BT2_MODULE)
	{
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2))
		{
			for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
			if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2))
			{
				bt_connect_status |= (1<<BT2_MODULE);
				return BT_MODULE_STATUS_CONNECTED;
			}
			else
			{
				bt_connect_status &= ~(1<<BT2_MODULE);
				return BT_MODULE_STATUS_DISCONNECT;
			}
		}
		else
		{
			bt_connect_status &= ~(1<<BT2_MODULE);
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (bt_channel == BT3_MODULE)
	{
		if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_14))
		{
			for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
			if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_14))
			{
				bt_connect_status |= (1<<BT3_MODULE);
				return BT_MODULE_STATUS_CONNECTED;
			}
			else
			{
				bt_connect_status &= ~(1<<BT3_MODULE);
				return BT_MODULE_STATUS_DISCONNECT;
			}
		}
		else
		{
			bt_connect_status &= ~(1<<BT3_MODULE);
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
#endif

#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (bt_channel == BT4_MODULE)
	{
		if(GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3))
		{
			for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
			if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_3))
			{
				bt_connect_status |= (1<<BT4_MODULE);
				return BT_MODULE_STATUS_CONNECTED;
			}
			else
			{
				bt_connect_status &= ~(1<<BT4_MODULE);
				return BT_MODULE_STATUS_DISCONNECT;
			}
		}
		else
		{
			bt_connect_status &= ~(1<<BT4_MODULE);
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
#endif

}

/*
 * @brief 通过蓝牙模块的透传模式发送数据到蓝牙主机
*/
void BT816_send_data(unsigned int bt_channel,unsigned char *data,unsigned int len)
{
	if (BT_CONNECT(bt_channel))
	{
		send_data_to_BT(bt_channel,data,len);
	}
}

/*
 * @brief 蓝牙模块BT816的初始化
*/
int BT816_init(void)
{
	unsigned char	str[21];
	int ret,i;
	for (i = 0; i < MAX_BT_CHANNEL;i++)
	{
		BT816_res[i].DataBuffer = BT816_recbuffer[i];
		BT816_reset_resVar(i);
		//初始化一个SPP的环形缓冲区
		ringbuffer_init(&spp_ringbuf[i],spp_rec_buffer[i],SPP_BUFFER_LEN);

		BT816_GPIO_config(i,115200);		//default波特率
		BT816_NVIC_config(i);
	}

	ret = BT816_Reset();
	if(ret < 0)
	{
		ret = BT816_Reset();
		if(ret < 0)
		{
			return -1;
		}
	}

#if 0
	if (BT816_query_name(BT1_MODULE,str))
	{
		return -4;
	}

	if (MEMCMP(str,"HJ Pr",5) != 0)
	{
		if (BT816_set_name(BT1_MODULE,"HJ Printer1"))
		{
			return -5;
		}
	}

	if (BT816_query_name(BT2_MODULE,str))
	{
		return -4;
	}

	if (MEMCMP(str,"HJ Pr",5) != 0)
	{
		if (BT816_set_name(BT2_MODULE,"HJ Printer2"))
		{
			return -5;
		}
	}

	if (BT816_query_name(BT3_MODULE,str))
	{
		return -4;
	}

	if (MEMCMP(str,"HJ Pr",5) != 0)
	{
		if (BT816_set_name(BT3_MODULE,"HJ Printer3"))
		{
			return -5;
		}
	}

	if (BT816_query_name(BT4_MODULE,str))
	{
		return -4;
	}

	if (MEMCMP(str,"HJ Pr",5) != 0)
	{
		if (BT816_set_name(BT4_MODULE,"HJ Printer4"))
		{
			return -5;
		}
	}

#endif

	MEMSET(spp_rec_buffer,0,MAX_BT_CHANNEL*SPP_BUFFER_LEN);
	RESET_BT1_DMA();
	RESET_BT2_DMA();
	RESET_BT3_DMA();
	RESET_BT4_DMA();
	return 0;
}

void set_BT_free(unsigned char ch)
{
	switch(ch)
	{
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	case BT1_MODULE:
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	break;
#endif
#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	case BT2_MODULE:
	GPIO_ResetBits(GPIOC, GPIO_Pin_1);
	break;
#endif
#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	case BT3_MODULE:
	GPIO_ResetBits(GPIOE, GPIO_Pin_15);
	break;
#endif
#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	case BT4_MODULE:
	GPIO_ResetBits(GPIOD, GPIO_Pin_0);
	break;
#endif
	default:
	break;
	}
}
#endif