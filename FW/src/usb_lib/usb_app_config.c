/**
* @file usb_config.c
* @brief 配置USB模式的模块
*
* @version V0.0.1
* @author kent.zhou
* @date 2015年11月16日
* @note
* 
* @copy
*
* 此代码为深圳合计电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*/

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "usb_app_config.h"
#include "stm32f10x_lib.h"
#include "hw_config.h"
#include "usb_init.h"
#include "TimeBase.h"
#include "usb_lib.h"
#include "usb_pwr.h"
#include "usb_desc.h"
#include "uart.h"


#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
unsigned char	g_mass_storage_device_type;
#ifdef DUMMY_FAT_FS
unsigned char	g_send_buff[512];
#endif
#endif


#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
unsigned char buffer_out[512];
unsigned int	count_out;
#endif

unsigned char				g_usb_type;
extern u32 count_in;


/*******************************************************************************
* Function Name  : Set_USBClock
* Description    : Configures USB Clock input (48MHz)
* Input          : None.
* Return         : None.
*******************************************************************************/
static void usb_SetClock(void)
{
	/* Enable GPIOA, GPIOD and USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD, ENABLE);

	/* USBCLK = PLLCLK / 1.5 */
	RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
	/* Enable USB clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}

/*******************************************************************************
* Function Name  : USB_Interrupts_Config
* Description    : Configures the USB interrupts
* Input          : None.
* Return         : None.
*******************************************************************************/
static void usb_Interrupts_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN_RX0_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USB_HP_CAN_TX_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : usb_cable_insert
* Description    : detect the USB Cable wether insert
* Input          : None.
* Return         : false: not insert true: insert  
*******************************************************************************/
bool usb_cable_insert (void)
{
	//@todo....
}


/**
* @brief 设备的初始化
* @param[in] #define USB_VIRTUAL_PORT		1	//虚拟串口
*			 #define USB_KEYBOARD			2	//USB键盘
*			 #define USB_MASSSTORAGE		3	//大容量存储设备
*			 #define USB_PRINTER			4	//USB打印机设备
* @note 函数调用者保证device_type确实是已经实现的USB类
*/
void usb_device_init(unsigned char device_type)
{
	if (device_type == USB_PRINTER)
	{
		MEMSET(spp_rec_buffer[USB_PRINT_CHANNEL_OFFSET],0,SPP_BUFFER_LEN);
		ringbuffer_init(&spp_ringbuf[USB_PRINT_CHANNEL_OFFSET],spp_rec_buffer[USB_PRINT_CHANNEL_OFFSET],SPP_BUFFER_LEN);
	}

	if (g_usb_type != device_type)
	{
		g_usb_type = device_type;
		usb_SetClock();
		usb_Interrupts_Config();
		USB_Init();
	}
}


void usb_SendData(unsigned char *pData, int length)
{
	volatile int i = 0;
        unsigned int	delay_cnt;
        
#if (USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)        
	int batch = length/VIRTUAL_COM_PORT_DATA_SIZE;
	int res	  = length%VIRTUAL_COM_PORT_DATA_SIZE;
#endif 
	

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	if (g_usb_type == USB_KEYBOARD)	//按键
	{
		count_in = length;
		UserToPMABufferCopy(pData, GetEPTxAddr(ENDP1), count_in);
		SetEPTxCount(ENDP1, count_in);
		/* enable endpoint for transmission */
		SetEPTxValid(ENDP1);
		//StartDelay(600);	//3S的延时
		delay_cnt = 300;
		while (count_in != 0 && delay_cnt != 0 &&(bDeviceState == CONFIGURED))
		{
			delay_cnt--;
			delay_ms(10);
		}
	}
	else 
#elif (USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if(g_usb_type == USB_VIRTUAL_PORT)
	{
		for (i = 0; i < batch; i++)
		{
			count_in	= VIRTUAL_COM_PORT_DATA_SIZE;
			UserToPMABufferCopy(pData+i*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, count_in);
			SetEPTxCount(ENDP1, count_in);
			SetEPTxValid(ENDP1);
			//StartDelay(600);
			delay_cnt = 300;
			while (count_in != 0 && delay_cnt != 0 && (bDeviceState == CONFIGURED))
			{
				delay_cnt--;
				delay_ms(10);
			}
		}
		if (res > 0)
		{
			//最后一包
			count_in	= res;
			UserToPMABufferCopy(pData+batch*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, count_in);
			SetEPTxCount(ENDP1, count_in);
			SetEPTxValid(ENDP1);
			//StartDelay(600);
			delay_cnt = 300;
			while (count_in != 0 && delay_cnt != 0 &&(bDeviceState == CONFIGURED))
			{
				delay_cnt--;
				delay_ms(10);
			}
		}
	}//Virtual port
	else 
#elif (USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
#ifdef DUMMY_FAT_FS
	if((g_usb_type == USB_MASSSTORAGE)&&(g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_DUMMY_FAT))
	{
		if (length > G_SEND_BUF_LENGTH)
		{
			length = G_SEND_BUF_LENGTH;
		}
		MEMCPY(g_send_buff,pData,length);
	}//USB_Masstorage
#endif
#endif
        ;
}

//int usb_test_connect(void)
//{
//	volatile int i = 0;
//	unsigned char pData[2] = {0};
//
//	count_in = 1;
//	UserToPMABufferCopy(pData, GetEPTxAddr(ENDP1), count_in);
//	/* enable endpoint for transmission */
//	SetEPTxValid(ENDP1);
//	while (count_in != 0 && i < 100000)
//	{
//		i++;
//	}
//	if (count_in == 0)
//	{
//		return 0;
//	}
//	return -1;
//}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void usb_Get_SerialNum(void)
{
	u32 Device_Serial0, Device_Serial1, Device_Serial2;

	Device_Serial0 = *(u32*)(0x1FFFF7E8);
	Device_Serial1 = *(u32*)(0x1FFFF7EC);
	Device_Serial2 = *(u32*)(0x1FFFF7F0);

	if (Device_Serial0 != 0)
	{
		USB_APP_StringSerial[2] = (u8)(Device_Serial0 & 0x000000FF);
		USB_APP_StringSerial[4] = (u8)((Device_Serial0 & 0x0000FF00) >> 8);
		USB_APP_StringSerial[6] = (u8)((Device_Serial0 & 0x00FF0000) >> 16);
		USB_APP_StringSerial[8] = (u8)((Device_Serial0 & 0xFF000000) >> 24);

		USB_APP_StringSerial[10] = (u8)(Device_Serial1 & 0x000000FF);
		USB_APP_StringSerial[12] = (u8)((Device_Serial1 & 0x0000FF00) >> 8);
		USB_APP_StringSerial[14] = (u8)((Device_Serial1 & 0x00FF0000) >> 16);
		USB_APP_StringSerial[16] = (u8)((Device_Serial1 & 0xFF000000) >> 24);

		USB_APP_StringSerial[18] = (u8)(Device_Serial2 & 0x000000FF);
		USB_APP_StringSerial[20] = (u8)((Device_Serial2 & 0x0000FF00) >> 8);
		USB_APP_StringSerial[22] = (u8)((Device_Serial2 & 0x00FF0000) >> 16);
		USB_APP_StringSerial[24] = (u8)((Device_Serial2 & 0xFF000000) >> 24);
	}
}