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
#include "ringbuffer.h"
#include "TimeBase.h"

//#define	BT816_DEBUG


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

TBT816Res		BT816_res;

#define SLEEP		1
#define WAKEUP		2

static unsigned char	BT816_send_buff[32];
static unsigned char	BT816_power_state;
unsigned char	BT816_recbuffer[BT816_RES_BUFFER_LEN];

#ifdef SPP_MODE
unsigned char	spp_rec_buffer[SPP_BUFFER_LEN];
struct ringbuffer	spp_ringbuf;
#endif


/*
 * @brief: 初始化模块端口
 * @note 使用串口2
*/
/*
 * @brief: 初始化模块端口
 * @note 使用串口2
*/
static void BT816_GPIO_config(unsigned int baudrate)
{
	GPIO_InitTypeDef				GPIO_InitStructure;
	USART_InitTypeDef				USART_InitStructure;
	DMA_InitTypeDef					DMA_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	//B-Reset  PD.1		B-Sleep	PD.0
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_1);
	GPIO_SetBits(GPIOD, GPIO_Pin_0);

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

	////trip for debug
	//GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_10;
	//GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	//GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	//GPIO_Init(GPIOC, &GPIO_InitStructure);
	//GPIO_SetBits(GPIOC, GPIO_Pin_10);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_10);


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
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer;  
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

	//使能通道6 
	DMA_Cmd(DMA2_Channel3,ENABLE);  

	//采用DMA方式接收  
	USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); 

	/* Enable UART4 DMA Tx request */
	USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);

	USART_Cmd(UART4, ENABLE);
}

/*
 * @brief: 串口中断的初始化
*/
static void BT816_NVIC_config(void)
{
	NVIC_InitTypeDef				NVIC_InitStructure;
	//中断配置  
	USART_ITConfig(UART4,USART_IT_TC,DISABLE);  
	USART_ITConfig(UART4,USART_IT_RXNE,DISABLE);  
	USART_ITConfig(UART4,USART_IT_IDLE,ENABLE);    

	//配置UART4中断  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;               //通道设置为串口2中断    
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;       //中断占先等级0    
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;              //中断响应优先级0    
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //打开中断    
	NVIC_Init(&NVIC_InitStructure);  

	/* Enable the DMA2 Channel5 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel4_5_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ITConfig(DMA2_Channel5, DMA_IT_TC | DMA_IT_TE, ENABLE);
	DMA_ClearFlag(DMA2_FLAG_TC5);
}


/**
* @brief  发数据给蓝牙模块
* @param[in] unsigned char *pData 要发送的数据
* @param[in] int length 要发送数据的长度
*/
static void send_data_to_BT816S01(const unsigned char *pData, unsigned int length)
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
	DMA_Cmd(DMA2_Channel5, DISABLE);

	/* set buffer address */
	memcpy(BT816_send_buff,pData,length);

	DMA2_Channel5->CMAR = (u32)&BT816_send_buff[0];
	/* set size */
	DMA2_Channel5->CNDTR = length;

	USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);
	/* enable DMA */
	DMA_Cmd(DMA2_Channel5, ENABLE);

	while(DMA2_Channel5->CNDTR);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET){};
}


/*
 * @brief 清空接收蓝牙模块响应数据的buffer
*/
static void BT816_reset_resVar(void)
{
	BT816_res.DataPos = 0;
	BT816_res.DataLength = 0;
	BT816_res.status	 = BT816_RES_INIT;
}


/**
* @brief 处理host收到BT816的数据
* @param[in] unsigned char c 读入的字符
* @return 0:success put in buffer
*        -1:fail
*/
int BT816_RxISRHandler(unsigned char *res, unsigned int res_len)
{	
	int i,len;
	if (res_len > 5)
	{
		BT816_res.DataLength = res_len;
		if ((res[0] == 0x0d)&&(res[1] == 0x0a)&&(res[2] == '+')			\
			&&(res[res_len-2] == 0x0d)&&(res[res_len-1] == 0x0a))
		{
#ifdef SPP_MODE
			if (res_len > 11)
			{
				if(memcmp(&res[3],"SPPREC=",7) == 0)
				{
					len = 0;
					for (i = 10; i < res_len-2;i++)
					{
						if (res[i] == ',')
						{
							break;
						}
						len*=10;
						len += res[i]-0x30;
					}

					if (len)
					{
						ringbuffer_put(&spp_ringbuf,&res[i+1],len);
					}

					return 0;
				}
			}

#endif

			if (BT816_res.status == BT816_RES_INIT)
			{
				for (i = 3; i < res_len-2;i++)
				{
					if (res[i] == '#')
					{
						if (res[i+1] == '0')
						{
							BT816_res.status = BT816_RES_SUCCESS;
						}
						else if (res[i+1] == '1')
						{
							BT816_res.status = BT816_RES_INVALID_STATE;
						}
						else if (res[i+1] == '2')
						{
							BT816_res.status = BT816_RES_INVALID_SYNTAX;
						}
						else
						{
							BT816_res.status = BT816_RES_BUSY;
						}
						break;
					}

					if (res[i] == '=')
					{
						BT816_res.status = BT816_RES_PAYLOAD;
						break;
					}
				}
			}
		}
		else
		{
			BT816_res.status = BT816_RES_UNKOWN;
		}
	}
        
        return 0;
}

#define EXPECT_RES_FORMAT1_TYPE		1
#define EXPECT_RES_FORMAT2_TYPE		2

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
static int BT816_write_cmd(const unsigned char *pData, unsigned int length,unsigned char type)
{
	unsigned int	wait_cnt;
	send_data_to_BT816S01(pData, length);
	BT816_reset_resVar();
	wait_cnt = 200;
	while (wait_cnt)
	{
		if (((BT816_res.status == BT816_RES_SUCCESS)&&(type == EXPECT_RES_FORMAT1_TYPE)) || ((BT816_res.status == BT816_RES_PAYLOAD)&&(type == EXPECT_RES_FORMAT2_TYPE)))
		{
			return 0;
		}
		else if (BT816_res.status == BT816_RES_INVALID_STATE || BT816_res.status == BT816_RES_INVALID_SYNTAX || BT816_res.status == BT816_RES_BUSY)
		{
			return -1;
		}
		else if (BT816_res.status == BT816_RES_UNKOWN)
		{
			return -2;
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
	//拉低复位信号100ms
	GPIO_ResetBits(GPIOD, GPIO_Pin_1);
	//OSTimeDlyHMSM(0,0,0,100);
	delay_ms(100);
    GPIO_SetBits(GPIOD, GPIO_Pin_1);

	BT816_reset_resVar();
	wait_cnt = 20;
	ret = 0;
	while (wait_cnt)
	{
		if (BT816_res.status == BT816_RES_PAYLOAD)
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
						if (memcmp(token,"BDTP",4)==0)
						{
							if (token_value[0] != '0')
							{
								ret |= 0x01;
							}
						}
#ifdef DEBUG_VER
						else if (memcmp(token,"BDVER",5)==0)
						{
							token_value[j]=0;
							printf("BlueTooth Module Ver:%s\r\n",token_value);
						}
						else if (memcmp(token,"BDADDR",6)==0)
						{
							token_value[j]=0;
							printf("BlueTooth Module Addr:%s\r\n",token_value);
						}
#endif
						else if (memcmp(token,"BDMODE",6)==0)
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
}

/*
 * @brief 查询蓝牙模块BT816的版本号
 * @param[out]  unsigned char *ver_buffer  返回查询到的版本号，如果为空表示查询失败
*/
int BT816_query_version(unsigned char *ver_buffer)
{
	unsigned char	buffer[21];
	int		i,ret;

	assert(ver_buffer != 0);
	ver_buffer[0] = 0;
	memcpy(buffer,"AT+BDVER=?\x0d\x0a",12);
	ret = BT816_write_cmd((const unsigned char*)buffer,12,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (memcmp(&BT816_res.DataBuffer[3],"BDVER",5) == 0)
	{
		for (i = 0; i < ((BT816_res.DataLength-11) > 20)?20:(BT816_res.DataLength-11);i++)
		{
			if (BT816_res.DataBuffer[9+i] == 0x0d)
			{
				break;
			}

			ver_buffer[i] = BT816_res.DataBuffer[9+i];
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
int BT816_query_name(unsigned char *name)
{
	unsigned char	buffer[15];
	int		i,ret;

	assert(name != 0);
	name[0] = 0;
	memcpy(buffer,"AT+BDNAME=?\x0d\x0a",13);

	ret = BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (memcmp(&BT816_res.DataBuffer[3],"BDNAME",6) == 0)
	{
		for (i = 0; i < ((BT816_res.DataLength-12) > 20)?20:(BT816_res.DataLength-12);i++)
		{
			if (BT816_res.DataBuffer[10+i] == 0x0d)
			{
				break;
			}

			name[i] = BT816_res.DataBuffer[10+i];
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
 *       在此接口中将设备名称限定为最长支持20个字节
*/
int BT816_set_name(unsigned char *name)
{
	unsigned char	buffer[33];
	int		len;

	assert(name != 0);
	memcpy(buffer,"AT+BDNAME=",10);
	len = strlen((char const*)name);
	if (len>20)
	{
		memcpy(buffer+10,name,20);
		buffer[30] = 0x0d;
		buffer[31] = 0x0a;
		len = 32;
	}
	else
	{
		memcpy(buffer+10,name,len);
		buffer[10+len] = 0x0d;
		buffer[11+len] = 0x0a;
		len += 12;
	}

	return BT816_write_cmd((const unsigned char*)buffer,len,EXPECT_RES_FORMAT2_TYPE); 
}

/*
 * @brief 设置蓝牙模块的设备名称
 * @param[in]  unsigned char *name  设置的名称,字符串
 * @return 0: 设置成功		else：设置失败
 * @note 从手册暂时没有看到支持的名称的最大长度是多少，所以如果设置的名称太长可能会设置失败
 *       在此接口中将设备名称限定为最长支持20个字节
*/
int BT816_set_baudrate(BT816_BAUDRATE baudrate)
{
	unsigned char	buffer[20];
	int		len;

	memcpy(buffer,"AT+BDBAUD=",10);
	len = 17;
	switch(baudrate)
	{
	case BAUDRATE_9600:
		memcpy(buffer+10,"9600",4);
		buffer[14] = 0x0d;
		buffer[15] = 0x0a;
		len = 16;
		break;
	case BAUDRATE_19200:
		memcpy(buffer+10,"19200",5);
		buffer[15] = 0x0d;
		buffer[16] = 0x0a;
		break;
	case BAUDRATE_38400:
		memcpy(buffer+10,"38400",5);
		buffer[15] = 0x0d;
		buffer[16] = 0x0a;
		break;
	case BAUDRATE_43000:
		memcpy(buffer+10,"43000",5);
		buffer[15] = 0x0d;
		buffer[16] = 0x0a;
		break;
	case BAUDRATE_57600:
		memcpy(buffer+10,"57600",5);
		buffer[15] = 0x0d;
		buffer[16] = 0x0a;
		break;
	case BAUDRATE_115200:
		memcpy(buffer+10,"115200",6);
		buffer[16] = 0x0d;
		buffer[17] = 0x0a;
		len = 18;
		break;
	}
	return BT816_write_cmd((const unsigned char*)buffer,len,EXPECT_RES_FORMAT2_TYPE);
}

/*
 * @brief 使蓝牙模块进入配对模式
 * @param[in]      
 * @return 0: 设置成功		else：设置失败
 * @note enter into pair mode will cause disconnection of current mode
*/
int BT816_enter_pair_mode(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+BDMODE=2\x0d\x0a",13);
	return BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT2_TYPE);
}

/*
 * @brief 设置蓝牙模块的HID键值传输的模式
 * @param[in]  unsigned char mode  蓝牙模块的HID键值传输模式     
 * @return 0: 设置成功		else：设置失败
*/
int BT816_set_hid_trans_mode(BT_HID_TRANS_MODE mode)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+BDTP=",8);
	if (mode == BT_HID_TRANS_MODE_AT)
	{
		buffer[8] = '0';
	}
	else
	{
		buffer[8] = '1';
	}
	buffer[9] = 0x0d;
	buffer[10] = 0x0a;
	//return  BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT2_TYPE);
	return  BT816_write_cmd((const unsigned char*)buffer,11,EXPECT_RES_FORMAT1_TYPE);
	//实测时发现此命令的响应为:+BDMODE#0,属于format1的响应
}
/*
 * @brief 设置蓝牙模块的工作模式(profile = HID、SPP、BLE)
 * @param[in]  unsigned char mode  蓝牙模块的工作模式     
 * @return 0: 设置成功		else：设置失败
*/
int BT816_set_profile(BT_PROFILE mode)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+BDMODE=",10);
	if (mode == BT_PROFILE_HID)
	{
		buffer[10] = '2';
	}
	else if (mode == BT_PROFILE_SPP)
	{
		buffer[10] = '1';
	}
	else
	{
		buffer[10] = '3';
	}
	buffer[11] = 0x0d;
	buffer[12] = 0x0a;
	//return  BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT2_TYPE);
	return  BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT1_TYPE);
	//实测时发现此命令的响应为:+BDMODE#0,属于format1的响应
}

/*
 * @brief 查询蓝牙模块HID当前的连接状态  	
 * @return <0 ：发送失败	0: unkown 	1: connected    2： disconnect
 * @note 
*/
int BT816_hid_status(void)
{
#if 0
	unsigned char	buffer[15];
	int		i,ret;

	memcpy(buffer,"AT+HIDSTAT=?\x0d\x0a",14);

	ret = BT816_write_cmd((const unsigned char*)buffer,14,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (memcmp(&BT816_res.DataBuffer[3],"HIDSTAT",7) == 0)
	{
		if (BT816_res.DataBuffer[11] == '3')
		{
			return BT_MODULE_STATUS_CONNECTED;
		}
		else
		{
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}

	return -2;
#endif

	unsigned int i;
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))
	{
		for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8))
		{
			return BT_MODULE_STATUS_CONNECTED;
		}
		else
		{
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
	else
		return BT_MODULE_STATUS_DISCONNECT;

}

/*
 * @brief 蓝牙模块试图连接最近一次连接过的主机
 * @return 0: 命令响应成功		else：命令响应失败
 * @note 由于此命令可能耗时比较长，所以此接口不等待连接的结果返回即退出
 *       如果需要知道是否连接成功，可以他通过查询状态的接口去获取
*/
int BT816_hid_connect_last_host(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDCONN\x0d\x0a",12);
	return BT816_write_cmd((const unsigned char*)buffer,12,EXPECT_RES_FORMAT1_TYPE);
}

/*
 * @brief 蓝牙模块试图断开与当前主机的连接
 * @return 0: 命令响应成功		else：命令响应失败
 * @note 由于此命令可能耗时比较长，所以此接口不等待断开的结果返回即退出
 *       如果需要知道是否断开成功，可以他通过查询状态的接口去获取
*/
int BT816_hid_disconnect(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDDISC\x0d\x0a",12);
	return BT816_write_cmd((const unsigned char*)buffer,12,EXPECT_RES_FORMAT1_TYPE);
}

/*
 * @brief 设置蓝牙模块是否使能IOS soft keyboard
 * @return 0: 设置成功		else：设置失败
*/
int BT816_toggle_ioskeypad(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDOSK\x0d\x0a",11);
	return BT816_write_cmd((const unsigned char*)buffer,11,EXPECT_RES_FORMAT2_TYPE);
}


#define ENTER_KEY             0x82
#define ESCAPE_KEY            0x83
#define BACKSPACE_KEY         0x84
#define TAB_KEY               0x85
#define SPACE_KEY             0x86
#define CAPS_LOCK_KEY         0x87

/*
 * @brief 通过蓝牙模块的HID模式发送ASCII字符串
 * @param[in]  unsigned char *str		需要发送的ASCII字符缓冲
 * @param[in]  unsigned int  len	    待发送字符数
 * @return 0: 发送成功		else：发送失败
 * @note   如果要发送的字符串太长，就会被拆包，其实理论上一次可以发送500字节，但是考虑到栈可能会溢出的风险，所以被拆包成40字节短包发送
 *         理论上可能会影响字符串的发送速度
*/
int BT816_hid_send(unsigned char *str,unsigned int len)
{
#if 1		//AT命令模式下发送键值
	unsigned char	buffer[60];
	unsigned char	str_len;
	unsigned char	*p;
	int ret;

	p = str;
	while (len > 40)
	{
		memcpy(buffer,"AT+HIDSEND=",11);
		hex_to_str(40,10,0,buffer+11);
		buffer[13]=',';
		memcpy(buffer+14,p,40);
		buffer[54]=0x0d;
		buffer[55]=0x0a;
		ret = BT816_write_cmd((const unsigned char*)buffer,56,EXPECT_RES_FORMAT1_TYPE);
		if (ret)
		{
			return ret;
		}
		len -= 40;
		p += 40;
	}

	memcpy(buffer,"AT+HIDSEND=",11);
	str_len = hex_to_str(len+2,10,0,buffer+11);
	buffer[11+str_len]=',';
	memcpy(buffer+12+str_len,p,len);
	buffer[12+str_len+len]=ENTER_KEY;

	buffer[13+str_len+len]=0x0d;
	buffer[14+str_len+len]=0x0a;
	ret = BT816_write_cmd((const unsigned char*)buffer,15+str_len+len,EXPECT_RES_FORMAT1_TYPE);
	return ret;
#endif

#if 0		//透传模式下发送键值
	send_data_to_BT816S01(str,len);
	send_data_to_BT816S01("\x0d\x0a",2);
	return 0;
#endif
}


/*
 * @brief 设置蓝牙模块是否使能自动连接特性
 * @param[in]	0: DIABLE		1:ENABLE
 * @return 0: 设置成功		else：设置失败
*/
int BT816_set_autocon(unsigned int	enable)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDACEN=",11);
	if (enable)
	{
		buffer[11] = '1';
	}
	else
	{
		buffer[11] = '0';
	}
	buffer[12] = 0x0d;
	buffer[13] = 0x0a;
	return  BT816_write_cmd((const unsigned char*)buffer,14,EXPECT_RES_FORMAT1_TYPE);	//实测返回的是format1的response
}


/*
 * @brief 设置蓝牙模块的HID传输延时
 * @param[in]	unsigned int	delay		单位ms
 * @return 0: 设置成功		else：设置失败
*/
int BT816_hid_set_delay(unsigned int	delay)
{
	unsigned char	buffer[20];
	int	len;

	memcpy(buffer,"AT+HIDSDLY=",11);
	len = hex_to_str(delay,10,0,buffer+11);
	buffer[11+len] = 0x0d;
	buffer[12+len] = 0x0a;
	return  BT816_write_cmd((const unsigned char*)buffer,13+len,EXPECT_RES_FORMAT2_TYPE);	//实测返回的是format1的response
}

/*
 * @brief 使蓝牙模块BT816进入睡眠模式
*/
void BT816_enter_sleep(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	BT816_power_state = SLEEP;
}

/*
 * @brief 唤醒蓝牙模块BT816
*/
void BT816_wakeup(void)
{
	if (BT816_power_state == SLEEP)
	{
		GPIO_SetBits(GPIOB,GPIO_Pin_12);
		BT816_power_state = WAKEUP;
		//OSTimeDlyHMSM(0,0,0,100);
                delay_ms(100);
	}
}

/*
 * @brief 查询蓝牙模块SPP当前的连接状态  	
 * @return <0 ：发送失败	0: unkown 	1: connected    2： disconnect
 * @note 
*/
int BT816_spp_status(void)
{
#if 0
	unsigned char	buffer[15];
	int		i,ret;

	memcpy(buffer,"AT+SPPSTAT=?\x0d\x0a",14);

	ret = BT816_write_cmd((const unsigned char*)buffer,14,EXPECT_RES_FORMAT2_TYPE);
	if (ret)
	{
		return ret;
	}

	if (memcmp(&BT816_res.DataBuffer[3],"SPPSTAT",7) == 0)
	{
		if (BT816_res.DataBuffer[11] == '3')
		{
			return BT_MODULE_STATUS_CONNECTED;
		}
		else
		{
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}

	return -2;
#endif

	unsigned int i;
	if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8))
	{
		for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
		if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8))
		{
			return BT_MODULE_STATUS_CONNECTED;
		}
		else
		{
			return BT_MODULE_STATUS_DISCONNECT;
		}
	}
	else
		return BT_MODULE_STATUS_DISCONNECT;

}

/*
 * @brief 蓝牙模块BT816的初始化
*/
int BT816_init(void)
{
	unsigned char	str[21];
	int ret;

	BT816_res.DataBuffer = BT816_recbuffer;
	BT816_reset_resVar();
        //初始化一个SPP的环形缓冲区
	ringbuffer_init(&spp_ringbuf,spp_rec_buffer,SPP_BUFFER_LEN);
        
	BT816_GPIO_config(115200);		//default波特率
	BT816_NVIC_config();
	ret = BT816_Reset();
	if(ret < 0)
	{
		ret = BT816_Reset();
		if(ret < 0)
		{
			return -1;
		}
	}

#ifdef HID_MODE
	if ((ret&0x02) == 0x02)
	{
		if (BT816_set_profile(BT_PROFILE_HID))
		{
			return -2;
		}
	}

	if ((ret & 0x01) == 1)
	{
		if (BT816_set_hid_trans_mode(BT_HID_TRANS_MODE_AT))
		{ 
			return -3;
		}
	}

	
	if (BT816_query_name(str))
	{
		return -4;
	}

	if (memcmp(str,"H520B",5) != 0)
	{
		if (BT816_set_name("H520B Device"))
		{
			return -5;
		}
	}
	
	if (BT816_set_autocon(1))
	{
		return -6;
	}

	if(BT816_hid_set_delay(8))
	{
		return -7;
	}
#else

	if (BT816_query_name(str))
	{
		return -4;
	}

	if (memcmp(str,"HJ Pr",5) != 0)
	{
		if (BT816_set_name("HJ Printer"))
		{
			return -5;
		}
	}

	//if (BT816_set_autocon(1))
	//{
	//	return -6;
	//}

	if ((ret&0x02) == 0x02)
	{
		if (BT816_set_profile(BT_PROFILE_SPP))
		{
			return -2;
		}
	}


	if ((ret & 0x01) == 1)
	{
		if (BT816_set_hid_trans_mode(BT_HID_TRANS_MODE_AT))
		{ 
			return -3;
		}
	}

	
#endif

	BT816_power_state = WAKEUP;
	return 0;
}


/*
 * @brief 蓝牙模块HID模式发送测试
*/
int BT816_hid_send_test(void)
{
	unsigned int i;
	for (i = 0; i < 50;i++)
	{
		BT816_hid_send("12345678901234567890",20);
		delay_ms(150);
	}
        
        return  0;
}
#endif