/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : hw_config.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Hardware Configuration & Setup
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "TimeBase.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//USART_InitTypeDef USART_InitStructure;
//ErrorStatus HSEStartUpStatus;

/* Extern variables ----------------------------------------------------------*/
//u8 buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];
extern u32 count_in;
extern LINE_CODING linecoding;




/*******************************************************************************
* Function Name  : Enter_LowPowerMode
* Description    : Power-off system clocks and power while entering suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
}

/*******************************************************************************
* Function Name  : Leave_LowPowerMode
* Description    : Restores system clocks and power while exiting suspend mode
* Input          : None.
* Return         : None.
*******************************************************************************/
void Leave_LowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }
}


/*******************************************************************************
* Function Name  : UART_To_USB_Send_Data.
* Description    : send the received data from UART 0 to USB.
* Input          : None.
* Return         : none.
*******************************************************************************/
//void USART_To_USB_Send_Data(void)
//{
//  if (linecoding.datatype == 7)
//  {
//    buffer_in[count_in] = USART_ReceiveData(USART1) & 0x7F;
//  }
//  else if (linecoding.datatype == 8)
//  {
//    buffer_in[count_in] = USART_ReceiveData(USART1);
//  }
//  count_in++;
//  UserToPMABufferCopy(buffer_in, ENDP1_TXADDR, count_in);
//  SetEPTxCount(ENDP1, count_in);
//  SetEPTxValid(ENDP1);
//}

void SendData_To_PC(unsigned char *pData, int length)
{
	int batch = length/VIRTUAL_COM_PORT_DATA_SIZE;
	int res	  = length%VIRTUAL_COM_PORT_DATA_SIZE;
	int i;

	for (i = 0; i < batch; i++)
	{
		count_in	= VIRTUAL_COM_PORT_DATA_SIZE;
		UserToPMABufferCopy(pData+i*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, count_in);
		SetEPTxCount(ENDP1, count_in);
		SetEPTxValid(ENDP1);
		StartDelay(600);
		while (count_in != 0 && DelayIsEnd() != 0)
		{
			;
		}
	}
	if (res > 0)
	{
		//最后一包
		count_in	= res;
		UserToPMABufferCopy(pData+batch*VIRTUAL_COM_PORT_DATA_SIZE, ENDP1_TXADDR, count_in);
		SetEPTxCount(ENDP1, count_in);
		SetEPTxValid(ENDP1);
		StartDelay(600);
		while (count_in != 0 && DelayIsEnd() != 0)
		{
			;
		}
	}
}

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Get_SerialNum(void)
{
  u32 Device_Serial0, Device_Serial1, Device_Serial2;

  Device_Serial0 = *(vu32*)(0x1FFFF7E8);
  Device_Serial1 = *(vu32*)(0x1FFFF7EC);
  Device_Serial2 = *(vu32*)(0x1FFFF7F0);

  if (Device_Serial0 != 0)
  {
    Virtual_Com_Port_StringSerial[2] = (u8)(Device_Serial0 & 0x000000FF);
    Virtual_Com_Port_StringSerial[4] = (u8)((Device_Serial0 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[6] = (u8)((Device_Serial0 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[8] = (u8)((Device_Serial0 & 0xFF000000) >> 24);

    Virtual_Com_Port_StringSerial[10] = (u8)(Device_Serial1 & 0x000000FF);
    Virtual_Com_Port_StringSerial[12] = (u8)((Device_Serial1 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[14] = (u8)((Device_Serial1 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[16] = (u8)((Device_Serial1 & 0xFF000000) >> 24);

    Virtual_Com_Port_StringSerial[18] = (u8)(Device_Serial2 & 0x000000FF);
    Virtual_Com_Port_StringSerial[20] = (u8)((Device_Serial2 & 0x0000FF00) >> 8);
    Virtual_Com_Port_StringSerial[22] = (u8)((Device_Serial2 & 0x00FF0000) >> 16);
    Virtual_Com_Port_StringSerial[24] = (u8)((Device_Serial2 & 0xFF000000) >> 24);
  }
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
