/**
* @file BT816.c
* @brief FSC BT816����ģ�������
* @version V0.0.1
* @author joe.zhou
* @date 2015��10��10��
* @note   ����ģ�����Ӧ�ص㣬������������ DMA���ͺͽ��� + ���ڿ����ж��ж���Ӧ���ݽ�����Ļ��ơ�
* @copy
*
* �˴���Ϊ���ںϽܵ������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
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


//Ӧ�����͵���Ӧ����״̬
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
* @brief BT816S01��Ӧ����  BT816S01->host
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
 * @brief: ��ʼ��ģ��˿�
 * @note ʹ�ô���2
*/
/*
 * @brief: ��ʼ��ģ��˿�
 * @note ʹ�ô���2
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

	// ʹ��UART4, PC11,PC10
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


	//DMA2ͨ��3����  
	DMA_DeInit(DMA2_Channel3);  
	//�����ַ  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);  
	//�ڴ��ַ  
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)BT816_recbuffer;  
	//dma���䷽����  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  
	//����DMA�ڴ���ʱ�������ĳ���  
	DMA_InitStructure.DMA_BufferSize = BT816_RES_BUFFER_LEN;  
	//����DMA���������ģʽ��һ������  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  
	//����DMA���ڴ����ģʽ  
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	//���������ֳ�  
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	//�ڴ������ֳ�  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;  
	//����DMA�Ĵ���ģʽ  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  
	//����DMA�����ȼ���  
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;  
	//����DMA��2��memory�еı����������  
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
	DMA_Init(DMA2_Channel3,&DMA_InitStructure);  

	//ʹ��ͨ��6 
	DMA_Cmd(DMA2_Channel3,ENABLE);  

	//����DMA��ʽ����  
	USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); 

	/* Enable UART4 DMA Tx request */
	USART_DMACmd(UART4, USART_DMAReq_Tx , ENABLE);

	USART_Cmd(UART4, ENABLE);
}

/*
 * @brief: �����жϵĳ�ʼ��
*/
static void BT816_NVIC_config(void)
{
	NVIC_InitTypeDef				NVIC_InitStructure;
	//�ж�����  
	USART_ITConfig(UART4,USART_IT_TC,DISABLE);  
	USART_ITConfig(UART4,USART_IT_RXNE,DISABLE);  
	USART_ITConfig(UART4,USART_IT_IDLE,ENABLE);    

	//����UART4�ж�  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQChannel;               //ͨ������Ϊ����2�ж�    
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;       //�ж�ռ�ȵȼ�0    
	//NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;              //�ж���Ӧ���ȼ�0    
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;                 //���ж�    
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
* @brief  �����ݸ�����ģ��
* @param[in] unsigned char *pData Ҫ���͵�����
* @param[in] int length Ҫ�������ݵĳ���
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
 * @brief ��ս�������ģ����Ӧ���ݵ�buffer
*/
static void BT816_reset_resVar(void)
{
	BT816_res.DataPos = 0;
	BT816_res.DataLength = 0;
	BT816_res.status	 = BT816_RES_INIT;
}


/**
* @brief ����host�յ�BT816������
* @param[in] unsigned char c ������ַ�
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
* @brief  �����������ģ��BT816S01,���ȴ���Ӧ���
* @param[in] unsigned char *pData Ҫ���͵�����
* @param[in] unsigned int	length Ҫ�������ݵĳ���
* @param[in] unsigned char  type   �ڴ���Ӧ���ݵ���������	
*							EXPECT_RES_FORMAT1_TYPE: response format1		
*							EXPECT_RES_FORMAT2_TYPE:response format2
* @return		0: �ɹ�
*				-1: ʧ��
*				-2: δ֪��Ӧ����������Ӧ����������BUG����Ҫ����
*				-3����Ӧ��ʱ
* @note	�ȴ�һ����Ӧ֡������
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
 * @brief ����ģ��BT816S01�ĸ�λ
*/
int BT816_Reset(void)
{
	unsigned int	wait_cnt,i,j;
	unsigned char	stat;
	int ret;
	//���͸�λ�ź�100ms
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
 * @brief ��ѯ����ģ��BT816�İ汾��
 * @param[out]  unsigned char *ver_buffer  ���ز�ѯ���İ汾�ţ����Ϊ�ձ�ʾ��ѯʧ��
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
 * @brief ��ѯ����ģ����豸����
 * @param[out]  unsigned char *name  ģ������,�ַ���
 * @return 0: ��ѯ�ɹ�		else����ѯʧ��
 * @note ���ֲ���ʱû�п���֧�ֵ����Ƶ���󳤶��Ƕ��٣�����������õ�����̫�����ܻᵼ�»��������
 *       �ڴ˽ӿ��н��豸�����޶�Ϊ�֧��20���ֽ�
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
 * @brief ��ѯ����������ģ����豸����
 * @param[in]  unsigned char *name  ���õ�����,�ַ���
 * @return 0: ���óɹ�		else������ʧ��
 * @note ���ֲ���ʱû�п���֧�ֵ����Ƶ���󳤶��Ƕ��٣�����������õ�����̫�����ܻ�����ʧ��
 *       �ڴ˽ӿ��н��豸�����޶�Ϊ�֧��20���ֽ�
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
 * @brief ��������ģ����豸����
 * @param[in]  unsigned char *name  ���õ�����,�ַ���
 * @return 0: ���óɹ�		else������ʧ��
 * @note ���ֲ���ʱû�п���֧�ֵ����Ƶ���󳤶��Ƕ��٣�����������õ�����̫�����ܻ�����ʧ��
 *       �ڴ˽ӿ��н��豸�����޶�Ϊ�֧��20���ֽ�
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
 * @brief ʹ����ģ��������ģʽ
 * @param[in]      
 * @return 0: ���óɹ�		else������ʧ��
 * @note enter into pair mode will cause disconnection of current mode
*/
int BT816_enter_pair_mode(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+BDMODE=2\x0d\x0a",13);
	return BT816_write_cmd((const unsigned char*)buffer,13,EXPECT_RES_FORMAT2_TYPE);
}

/*
 * @brief ��������ģ���HID��ֵ�����ģʽ
 * @param[in]  unsigned char mode  ����ģ���HID��ֵ����ģʽ     
 * @return 0: ���óɹ�		else������ʧ��
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
	//ʵ��ʱ���ִ��������ӦΪ:+BDMODE#0,����format1����Ӧ
}
/*
 * @brief ��������ģ��Ĺ���ģʽ(profile = HID��SPP��BLE)
 * @param[in]  unsigned char mode  ����ģ��Ĺ���ģʽ     
 * @return 0: ���óɹ�		else������ʧ��
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
	//ʵ��ʱ���ִ��������ӦΪ:+BDMODE#0,����format1����Ӧ
}

/*
 * @brief ��ѯ����ģ��HID��ǰ������״̬  	
 * @return <0 ������ʧ��	0: unkown 	1: connected    2�� disconnect
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
		for (i=0;i < 2000;i++);		//��ʱһС��ʱ�䣬��ֹ����Ϊ������ɵ�
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
 * @brief ����ģ����ͼ�������һ�����ӹ�������
 * @return 0: ������Ӧ�ɹ�		else��������Ӧʧ��
 * @note ���ڴ�������ܺ�ʱ�Ƚϳ������Դ˽ӿڲ��ȴ����ӵĽ�����ؼ��˳�
 *       �����Ҫ֪���Ƿ����ӳɹ���������ͨ����ѯ״̬�Ľӿ�ȥ��ȡ
*/
int BT816_hid_connect_last_host(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDCONN\x0d\x0a",12);
	return BT816_write_cmd((const unsigned char*)buffer,12,EXPECT_RES_FORMAT1_TYPE);
}

/*
 * @brief ����ģ����ͼ�Ͽ��뵱ǰ����������
 * @return 0: ������Ӧ�ɹ�		else��������Ӧʧ��
 * @note ���ڴ�������ܺ�ʱ�Ƚϳ������Դ˽ӿڲ��ȴ��Ͽ��Ľ�����ؼ��˳�
 *       �����Ҫ֪���Ƿ�Ͽ��ɹ���������ͨ����ѯ״̬�Ľӿ�ȥ��ȡ
*/
int BT816_hid_disconnect(void)
{
	unsigned char	buffer[15];

	memcpy(buffer,"AT+HIDDISC\x0d\x0a",12);
	return BT816_write_cmd((const unsigned char*)buffer,12,EXPECT_RES_FORMAT1_TYPE);
}

/*
 * @brief ��������ģ���Ƿ�ʹ��IOS soft keyboard
 * @return 0: ���óɹ�		else������ʧ��
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
 * @brief ͨ������ģ���HIDģʽ����ASCII�ַ���
 * @param[in]  unsigned char *str		��Ҫ���͵�ASCII�ַ�����
 * @param[in]  unsigned int  len	    �������ַ���
 * @return 0: ���ͳɹ�		else������ʧ��
 * @note   ���Ҫ���͵��ַ���̫�����ͻᱻ�������ʵ������һ�ο��Է���500�ֽڣ����ǿ��ǵ�ջ���ܻ�����ķ��գ����Ա������40�ֽڶ̰�����
 *         �����Ͽ��ܻ�Ӱ���ַ����ķ����ٶ�
*/
int BT816_hid_send(unsigned char *str,unsigned int len)
{
#if 1		//AT����ģʽ�·��ͼ�ֵ
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

#if 0		//͸��ģʽ�·��ͼ�ֵ
	send_data_to_BT816S01(str,len);
	send_data_to_BT816S01("\x0d\x0a",2);
	return 0;
#endif
}


/*
 * @brief ��������ģ���Ƿ�ʹ���Զ���������
 * @param[in]	0: DIABLE		1:ENABLE
 * @return 0: ���óɹ�		else������ʧ��
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
	return  BT816_write_cmd((const unsigned char*)buffer,14,EXPECT_RES_FORMAT1_TYPE);	//ʵ�ⷵ�ص���format1��response
}


/*
 * @brief ��������ģ���HID������ʱ
 * @param[in]	unsigned int	delay		��λms
 * @return 0: ���óɹ�		else������ʧ��
*/
int BT816_hid_set_delay(unsigned int	delay)
{
	unsigned char	buffer[20];
	int	len;

	memcpy(buffer,"AT+HIDSDLY=",11);
	len = hex_to_str(delay,10,0,buffer+11);
	buffer[11+len] = 0x0d;
	buffer[12+len] = 0x0a;
	return  BT816_write_cmd((const unsigned char*)buffer,13+len,EXPECT_RES_FORMAT2_TYPE);	//ʵ�ⷵ�ص���format1��response
}

/*
 * @brief ʹ����ģ��BT816����˯��ģʽ
*/
void BT816_enter_sleep(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	BT816_power_state = SLEEP;
}

/*
 * @brief ��������ģ��BT816
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
 * @brief ��ѯ����ģ��SPP��ǰ������״̬  	
 * @return <0 ������ʧ��	0: unkown 	1: connected    2�� disconnect
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
		for (i=0;i < 2000;i++);		//��ʱһС��ʱ�䣬��ֹ����Ϊ������ɵ�
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
 * @brief ����ģ��BT816�ĳ�ʼ��
*/
int BT816_init(void)
{
	unsigned char	str[21];
	int ret;

	BT816_res.DataBuffer = BT816_recbuffer;
	BT816_reset_resVar();
        //��ʼ��һ��SPP�Ļ��λ�����
	ringbuffer_init(&spp_ringbuf,spp_rec_buffer,SPP_BUFFER_LEN);
        
	BT816_GPIO_config(115200);		//default������
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
 * @brief ����ģ��HIDģʽ���Ͳ���
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