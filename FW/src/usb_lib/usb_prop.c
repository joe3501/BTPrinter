/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_prop.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : All processings related to Joystick Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "hw_config.h"
#include "usb_bot.h"
#include "usb_app_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
unsigned int	keyboard_output_report_value;
u32 ProtocolValue;
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
u32 Max_Lun = 0;
extern u8 Bot_State;
extern Bulk_Only_CBW CBW;
static u8 *Get_Max_Lun(u16 Length);
#endif


#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
u8 Request = 0;

LINE_CODING linecoding =
{
	115200, /* baud rate*/
	0x00,   /* stop bits-1*/
	0x00,   /* parity - none*/
	0x08    /* no. of bits 8*/
};
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
u8 port_status;
const u8 *device_id="MFG:EPSON;CMD:EJL,ESCP24J-84,ESCPAGEJ-04,ESCPSUPER-00;MDL:LP-2400;CLS:PRINTER;DES:HJ KT486;";

static u8 *Get_Device_id(u16 Length);
static u8 *Get_Port_status(u16 Length);
static void print_device_reset(void);
#endif

/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    USB_APP_init,
    USB_APP_Reset,
    USB_APP_Status_In,
    USB_APP_Status_Out,
    USB_APP_Data_Setup,
    USB_APP_NoData_Setup,
    USB_APP_Get_Interface_Setting,
    USB_APP_GetDeviceDescriptor,
    USB_APP_GetConfigDescriptor,
    USB_APP_GetStringDescriptor,
    0,
	0x40 /*MAX PACKET SIZE*/
  };
USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    USB_APP_GetConfiguration,
    USB_APP_SetConfiguration,
    USB_APP_GetInterface,
    USB_APP_SetInterface,
    USB_APP_GetStatus,
    USB_APP_ClearFeature,
    USB_APP_SetEndPointFeature,
    USB_APP_SetDeviceFeature,
    USB_APP_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor[USB_DEVICE_TYPE_NUM] =
  {
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	  {
		  (u8*)Virtual_Com_Port_DeviceDescriptor,
			  USB_APP_SIZ_DEVICE_DESC
	  },
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	  {
		  (u8*)Keyboard_DeviceDescriptor,
		  USB_APP_SIZ_DEVICE_DESC
	  },
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	 {
		(u8*)MASS_DeviceDescriptor,
		MASS_SIZ_DEVICE_DESC
	 },
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
		  {
			  (u8*)Printer_DeviceDescriptor,
				  PRINTER_SIZ_DEVICE_DESC
		  }
#endif
  };

ONE_DESCRIPTOR Config_Descriptor[USB_DEVICE_TYPE_NUM] =
  {
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	  {
		  (u8*)Virtual_Com_Port_ConfigDescriptor,
			  VIRTUAL_COM_PORT_SIZ_CONFIG_DESC
	  },
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	  {
		(u8*)Keyboard_ConfigDescriptor,
		KEYBOARD_SIZ_CONFIG_DESC
	  },
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	{
		(u8*)MASS_ConfigDescriptor,
			MASS_SIZ_CONFIG_DESC
	},
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
		{
			(u8*)Printer_ConfigDescriptor,
				PRINTER_SIZ_CONFIG_DESC
		}
#endif
  };

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
ONE_DESCRIPTOR Keyboard_Report_Descriptor =
  {
    (u8 *)Keyboard_ReportDescriptor,
    KEYBOARD_SIZ_REPORT_DESC
  };

ONE_DESCRIPTOR Keyboard_Hid_Descriptor =
  {
    (u8*)Keyboard_ConfigDescriptor + KEYBOARD_OFF_HID_DESC,
    KEYBOARD_SIZ_HID_DESC
  };
#endif


ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(u8*)USB_APP_StringLangID, USB_APP_SIZ_STRING_LANGID},
    {(u8*)USB_APP_StringVendor, USB_APP_SIZ_STRING_VENDOR},
    {(u8*)USB_APP_StringProduct, USB_APP_SIZ_STRING_PRODUCT},
    {(u8*)USB_APP_StringSerial, USB_APP_SIZ_STRING_SERIAL}
  };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void USB_Set_Descriptor(void)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		Device_Descriptor[VIRTUAL_COMM_DESC_OFFSET].Descriptor =  (u8*)Virtual_Com_Port_DeviceDescriptor;

		Config_Descriptor[VIRTUAL_COMM_DESC_OFFSET].Descriptor = (u8*)Virtual_Com_Port_ConfigDescriptor;
		Config_Descriptor[VIRTUAL_COMM_DESC_OFFSET].Descriptor_Size	= VIRTUAL_COM_PORT_SIZ_CONFIG_DESC;
	}
	else 
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	if(g_usb_type == USB_KEYBOARD)
	{
		Device_Descriptor[KEYBOARD_DESC_OFFSET].Descriptor = (u8*)Keyboard_DeviceDescriptor;

		Config_Descriptor[KEYBOARD_DESC_OFFSET].Descriptor = (u8*)Keyboard_ConfigDescriptor;
		Config_Descriptor[KEYBOARD_DESC_OFFSET].Descriptor_Size	= KEYBOARD_SIZ_CONFIG_DESC;
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if(g_usb_type == USB_MASSSTORAGE)
	{
		Device_Descriptor[MASS_DESC_OFFSET].Descriptor = (u8*)MASS_DeviceDescriptor;

		Config_Descriptor[MASS_DESC_OFFSET].Descriptor = (u8*)MASS_ConfigDescriptor;
		Config_Descriptor[MASS_DESC_OFFSET].Descriptor_Size	= MASS_SIZ_CONFIG_DESC;
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
	{
		Device_Descriptor[PRINTER_DESC_OFFSET].Descriptor = (u8*)Printer_DeviceDescriptor;

		Config_Descriptor[PRINTER_DESC_OFFSET].Descriptor = (u8*)Printer_ConfigDescriptor;
		Config_Descriptor[PRINTER_DESC_OFFSET].Descriptor_Size	= PRINTER_SIZ_CONFIG_DESC;
	}
#endif
      ;
}

/*******************************************************************************
* Function Name  : Joystick_init.
* Description    : Joystick Mouse init routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_init(void)
{

  /* Update the serial number string descriptor with the data from the unique
  ID*/
  usb_Get_SerialNum();

  pInformation->Current_Configuration = 0;
  /* Connect the device */
  USB_PowerOn();
  /* USB interrupts initialization */
  _SetISTR(0);               /* clear pending interrupts */
  wInterrupt_Mask = IMR_MSK;
  _SetCNTR(wInterrupt_Mask); /* set interrupts mask */

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : Joystick_Reset.
* Description    : Joystick Mouse reset routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_Reset(void)
{
  /* Set Joystick_DEVICE as not configured */
  pInformation->Current_Configuration = 0;
 
  /* Current Feature initialization */
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
  if (g_usb_type == USB_VIRTUAL_PORT)
  {
	  pInformation->Current_Interface = 0;
	  pInformation->Current_Feature = Virtual_Com_Port_ConfigDescriptor[7];
  }
  else 
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	  if(g_usb_type == USB_KEYBOARD)
  {
	 pInformation->Current_Interface = 0;
	 pInformation->Current_Feature = Keyboard_ConfigDescriptor[7];
  }
  else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
  {
	pInformation->Current_Feature = MASS_ConfigDescriptor[7];
  }
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
	  {
		  pInformation->Current_Interface = 0;
		  pInformation->Current_Feature = Printer_ConfigDescriptor[7];
	  }
#endif

  SetBTABLE(BTABLE_ADDRESS);

  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
if (g_usb_type == USB_VIRTUAL_PORT)
{
	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_DIS);

	/* Initialize Endpoint 2 */
	SetEPType(ENDP2, EP_INTERRUPT);
	SetEPTxAddr(ENDP2, ENDP2_TXADDR);
	SetEPRxStatus(ENDP2, EP_RX_DIS);
	SetEPTxStatus(ENDP2, EP_TX_NAK);

	/* Initialize Endpoint 3 */
	SetEPType(ENDP3, EP_BULK);
	SetEPRxAddr(ENDP3, ENDP3_RXADDR);
	SetEPRxCount(ENDP3, VIRTUAL_COM_PORT_DATA_SIZE);
	SetEPRxStatus(ENDP3, EP_RX_VALID);
	SetEPTxStatus(ENDP3, EP_TX_DIS);
}
else 
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
if(g_usb_type == USB_KEYBOARD)
{
	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_INTERRUPT);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxCount(ENDP1, 1);
	SetEPRxStatus(ENDP1, EP_RX_DIS);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
}
else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
{
	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_DIS);

	/* Initialize Endpoint 2 */
	SetEPType(ENDP2, EP_BULK);
	SetEPRxAddr(ENDP2, ENDP2_RXADDR);
	SetEPRxCount(ENDP2, Device_Property.MaxPacketSize);			
	//SetEPRxCount(ENDP2, 0x40);										//joe ÐÞ¸Ä
	SetEPRxStatus(ENDP2, EP_RX_VALID);
	SetEPTxStatus(ENDP2, EP_TX_DIS);


	SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
	SetEPRxValid(ENDP0);

	CBW.dSignature = BOT_CBW_SIGNATURE;
	Bot_State = BOT_IDLE;
}
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
{
	/* Initialize Endpoint 1 */
	SetEPType(ENDP1, EP_BULK);
	SetEPTxAddr(ENDP1, ENDP1_TXADDR);
	SetEPTxStatus(ENDP1, EP_TX_NAK);
	SetEPRxStatus(ENDP1, EP_RX_DIS);

	/* Initialize Endpoint 2 */
	SetEPType(ENDP2, EP_BULK);
	SetEPRxAddr(ENDP2, ENDP2_RXADDR);
	SetEPRxCount(ENDP2, Device_Property.MaxPacketSize);			
	//SetEPRxCount(ENDP2, 0x40);										//joe ÐÞ¸Ä
	SetEPRxStatus(ENDP2, EP_RX_VALID);
	SetEPTxStatus(ENDP2, EP_TX_DIS);
}
#endif

  bDeviceState = ATTACHED;

  /* Set this device to response on default address */
  SetDeviceAddress(0);
}
/*******************************************************************************
* Function Name  : Joystick_SetConfiguration.
* Description    : Udpade the device state to configured.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_SetConfiguration(void)
{
  DEVICE_INFO *pInfo = &Device_Info;

  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if (g_usb_type == USB_MASSSTORAGE)
	{
		ClearDTOG_TX(ENDP1);
		ClearDTOG_RX(ENDP2);

		Bot_State = BOT_IDLE; /* set the Bot state machine to the IDLE state */
	}
#endif

  }
}

/*******************************************************************************
* Function Name  : Mass_Storage_ClearFeature
* Description    : Handle the ClearFeature request.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_ClearFeature(void)
{
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	/* when the host send a CBW with invalid signature or invalid length the two
	Endpoints (IN & OUT) shall stall until receiving a Mass Storage Reset     */
	if (g_usb_type == USB_MASSSTORAGE)
	{
		if (CBW.dSignature != BOT_CBW_SIGNATURE)
			Bot_Abort(BOTH_DIR);
	}
#endif

}

/*******************************************************************************
* Function Name  : Joystick_SetConfiguration.
* Description    : Udpade the device state to addressed.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}
/*******************************************************************************
* Function Name  : Joystick_Status_In.
* Description    : Joystick status IN routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_Status_In(void)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		if (Request == SET_LINE_CODING)
		{
			//USART_Config();
			Request = 0;
		}
	}
#endif
}

/*******************************************************************************
* Function Name  : Joystick_Status_Out
* Description    : Joystick status OUT routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void USB_APP_Status_Out (void)
{}

/*******************************************************************************
* Function Name  : Joystick_Data_Setup
* Description    : Handle the data class specific requests.
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT USB_APP_Data_Setup(u8 RequestNo)
{
  u8 *(*CopyRoutine)(u16);

  CopyRoutine = NULL;
 #if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
  if (g_usb_type == USB_VIRTUAL_PORT)
  {
	  if (RequestNo == GET_LINE_CODING)
	  {
		  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
		  {
			  CopyRoutine = Virtual_Com_Port_GetLineCoding;
		  }
	  }
	  else if (RequestNo == SET_LINE_CODING)
	  {
		  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
		  {
			  CopyRoutine = Virtual_Com_Port_SetLineCoding;
		  }
		  Request = SET_LINE_CODING;
	  }
  }
  else 
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	  if(g_usb_type == USB_KEYBOARD)
  {
	  if ((RequestNo == GET_DESCRIPTOR)
		  && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
		  && (pInformation->USBwIndex0 == 0))
	  {

		  if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
		  {
			  CopyRoutine = Keyboard_GetReportDescriptor;
		  }
		  else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
		  {
			  CopyRoutine = Keyboard_GetHIDDescriptor;
		  }

	  } /* End of GET_DESCRIPTOR */

	  /*** GET_PROTOCOL ***/
	  else if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
	  {
		  if(RequestNo == GET_PROTOCOL)
		  {
			  CopyRoutine = Keyboard_GetProtocolValue;
		  }
		  else if (RequestNo == SET_REPORT)
		  {
			 CopyRoutine = Keyboard_SetReport;
		  }
	  }
  }
  else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	   if(g_usb_type == USB_MASSSTORAGE)
  {
	  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
		  && (RequestNo == GET_MAX_LUN) && (pInformation->USBwValue == 0)
		  && (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x01))
	  {
		  CopyRoutine = Get_Max_Lun;
	  }
	  else
	  {
		  return USB_UNSUPPORT;
	  }
  }
	   else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
	  {
		  if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT)))
		  {
			  if ((RequestNo == GET_DEVICE_ID) && (pInformation->USBwValue == 0)
				  && (pInformation->USBwIndex == 0))
			{
			  //CopyRoutine = Get_Device_id;
			}
			  else if ((RequestNo == GET_PORT_STATUS) && (pInformation->USBwValue == 0)
				  && (pInformation->USBwIndex == 0)&&(pInformation->USBwLength == 1))
			  {
					CopyRoutine = Get_Port_status;
			  }
			  else
			  {
				return USB_UNSUPPORT;
			  }
		  }
		  else
		  {
			  return USB_UNSUPPORT;
		  }
	  }
#endif
	;
  if (CopyRoutine == NULL)
  {
	  return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Joystick_NoData_Setup
* Description    : handle the no data class specific requests
* Input          : Request Nb.
* Output         : None.
* Return         : USB_UNSUPPORT or USB_SUCCESS.
*******************************************************************************/
RESULT USB_APP_NoData_Setup(u8 RequestNo)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
		{
			if (RequestNo == SET_COMM_FEATURE)
			{
				return USB_SUCCESS;
			}
			else if (RequestNo == SET_CONTROL_LINE_STATE)
			{
				return USB_SUCCESS;
			}
		}
	}
	else 
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
		if(g_usb_type == USB_KEYBOARD)
	{
		if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			&& (RequestNo == SET_PROTOCOL))
		{
			return Keyboard_SetProtocol();
		}
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
		if(g_usb_type == USB_MASSSTORAGE)
	{
		if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
			&& (RequestNo == MASS_STORAGE_RESET) && (pInformation->USBwValue == 0)
			&& (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00))
		{

			/* Initialize Endpoint 1 */
			ClearDTOG_TX(ENDP1);

			/* Initialize Endpoint 2 */
			ClearDTOG_RX(ENDP2);

			/*intialise the CBW signature to enable the clear feature*/
			CBW.dSignature = BOT_CBW_SIGNATURE;
			Bot_State = BOT_IDLE;

			return USB_SUCCESS;
		}
	}
		else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
			{
				if ((Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
					&& (RequestNo == SOFT_RESET) && (pInformation->USBwValue == 0)
					&& (pInformation->USBwIndex == 0) && (pInformation->USBwLength == 0x00))
				{
					print_device_reset();
					return USB_SUCCESS;
				}
			}
#endif
        ;
	return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : Joystick_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the device descriptor.
*******************************************************************************/
u8 *USB_APP_GetDeviceDescriptor(u16 Length)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		return Standard_GetDescriptorData(Length, &Device_Descriptor[VIRTUAL_COMM_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	if(g_usb_type == USB_KEYBOARD)
	{
		return Standard_GetDescriptorData(Length, &Device_Descriptor[KEYBOARD_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if(g_usb_type == USB_MASSSTORAGE)
	{
		return Standard_GetDescriptorData(Length, &Device_Descriptor[MASS_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
		if(g_usb_type == USB_PRINTER)
		{
			return Standard_GetDescriptorData(Length, &Device_Descriptor[PRINTER_DESC_OFFSET]);
		}
#endif
        ;
}

/*******************************************************************************
* Function Name  : Joystick_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
u8 *USB_APP_GetConfigDescriptor(u16 Length)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		return Standard_GetDescriptorData(Length, &Config_Descriptor[VIRTUAL_COMM_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
	if(g_usb_type == USB_KEYBOARD)
	{
		return Standard_GetDescriptorData(Length, &Config_Descriptor[KEYBOARD_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if(g_usb_type == USB_MASSSTORAGE)
	{
		return Standard_GetDescriptorData(Length, &Config_Descriptor[MASS_DESC_OFFSET]);
	}
	else
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
		if(g_usb_type == USB_PRINTER)
		{
			return Standard_GetDescriptorData(Length, &Config_Descriptor[PRINTER_DESC_OFFSET]);
		}
#endif
        ;
}

/*******************************************************************************
* Function Name  : Joystick_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
* Input          : Length
* Output         : None.
* Return         : The address of the string descriptors.
*******************************************************************************/
u8 *USB_APP_GetStringDescriptor(u16 Length)
{
  u8 wValue0 = pInformation->USBwValue0;
  if (wValue0 > 4)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
/*******************************************************************************
* Function Name  : Joystick_GetReportDescriptor.
* Description    : Gets the HID report descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
u8 *Keyboard_GetReportDescriptor(u16 Length)
{
  return Standard_GetDescriptorData(Length, &Keyboard_Report_Descriptor);
}

/*******************************************************************************
* Function Name  : Joystick_GetHIDDescriptor.
* Description    : Gets the HID descriptor.
* Input          : Length
* Output         : None.
* Return         : The address of the configuration descriptor.
*******************************************************************************/
u8 *Keyboard_GetHIDDescriptor(u16 Length)
{
  return Standard_GetDescriptorData(Length, &Keyboard_Hid_Descriptor);
}
#endif

/*******************************************************************************
* Function Name  : Joystick_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
* Input          : - Interface : interface number.
*                  - AlternateSetting : Alternate Setting number.
* Output         : None.
* Return         : USB_SUCCESS or USB_UNSUPPORT.
*******************************************************************************/
RESULT USB_APP_Get_Interface_Setting(u8 Interface, u8 AlternateSetting)
{
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
	if (g_usb_type == USB_VIRTUAL_PORT)
	{
		if (AlternateSetting > 0)
		{
			return USB_UNSUPPORT;
		}
		else if (Interface > 1)
		{
			return USB_UNSUPPORT;
		}
	}
	else
#endif
	{
		if (AlternateSetting > 0)
		{
			return USB_UNSUPPORT;
		}
		else if (Interface > 0)
		{
			return USB_UNSUPPORT;
		}
	}

  return USB_SUCCESS;
}


#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
/*******************************************************************************
* Function Name  : Joystick_SetProtocol
* Description    : Joystick Set Protocol request routine.
* Input          : None.
* Output         : None.
* Return         : USB SUCCESS.
*******************************************************************************/
RESULT Keyboard_SetProtocol(void)
{
  u8 wValue0 = pInformation->USBwValue0;
  ProtocolValue = wValue0;
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : Joystick_GetProtocolValue
* Description    : get the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the protcol value.
*******************************************************************************/
u8 *Keyboard_GetProtocolValue(u16 Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  else
  {
    return (u8 *)(&ProtocolValue);
  }
}

/*******************************************************************************
* Function Name  : Joystick_SetReportValue
* Description    : set the protocol value
* Input          : Length.
* Output         : None.
* Return         : address of the set report value.
*******************************************************************************/
u8 *Keyboard_SetReport(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = sizeof(keyboard_output_report_value);
		return NULL;
	}
	return(u8 *)&keyboard_output_report_value;
}
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
/*******************************************************************************
* Function Name  : Virtual_Com_Port_GetLineCoding.
* Description    : send the linecoding structure to the PC host.
* Input          : Length.
* Output         : None.
* Return         : Inecoding structure base address.
*******************************************************************************/
u8 *Virtual_Com_Port_GetLineCoding(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
		return NULL;
	}
	return(u8 *)&linecoding;
}

/*******************************************************************************
* Function Name  : Virtual_Com_Port_SetLineCoding.
* Description    : Set the linecoding structure fields.
* Input          : Length.
* Output         : None.
* Return         : Linecoding structure base address.
*******************************************************************************/
u8 *Virtual_Com_Port_SetLineCoding(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
		return NULL;
	}
	return(u8 *)&linecoding;
}
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
/*******************************************************************************
* Function Name  : Get_Max_Lun
* Description    : Handle the Get Max Lun request.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
static u8 *Get_Max_Lun(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = LUN_DATA_LENGTH;
		return 0;
	}
	else
	{
		return((u8*)(&Max_Lun));
	}
}
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
/*******************************************************************************
* Function Name  : Get_Device_id
* Description    : Handle the Get Max Lun request.
* Input          : uint16_t Length.
* Output         : None.
* Return         : None.
*******************************************************************************/
static u8 *Get_Device_id(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = strlen(device_id);
		return 0;
	}
	else
	{
		return((u8*)(device_id));
	}
}

static u8 *Get_Port_status(u16 Length)
{
	if (Length == 0)
	{
		pInformation->Ctrl_Info.Usb_wLength = PORT_STATUS_DATA_LEN;
		return 0;
	}
	else
	{
		port_status = 0x18;
		if (esc_sts[USB_PRINT_CHANNEL_OFFSET].status4&(0x03<<5))
		{
			//È±Ö½
			port_status |= (0x01<<5); 
		}
		return((u8*)(&port_status));
	}
}

//soft reset
static void print_device_reset(void)
{
	current_channel = USB_PRINT_CHANNEL_OFFSET;
	esc_p_init(USB_PRINT_CHANNEL_OFFSET);
	PrintBufToZero();
}
#endif

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
