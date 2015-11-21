/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_prop.h
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : All processings related to Joystick Mouse demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PROP_H
#define __USB_PROP_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum _HID_REQUESTS
{
  GET_REPORT = 1,
  GET_IDLE,
  GET_PROTOCOL,

  SET_REPORT = 9,
  SET_IDLE,
  SET_PROTOCOL
} HID_REQUESTS;

typedef struct
{
	u32 bitrate;
	u8 format;
	u8 paritytype;
	u8 datatype;
}LINE_CODING;

/* Exported define -----------------------------------------------------------*/
#define USB_APP_GetConfiguration          NOP_Process
//#define USB_SetConfiguration          NOP_Process
#define USB_APP_GetInterface              NOP_Process
#define USB_APP_SetInterface              NOP_Process
#define USB_APP_GetStatus                 NOP_Process
//#define USB_APP_ClearFeature              NOP_Process
#define USB_APP_SetEndPointFeature        NOP_Process
#define USB_APP_SetDeviceFeature          NOP_Process
//#define USB_SetDeviceAddress          NOP_Process

#define REPORT_DESCRIPTOR                  0x22

#define GET_MAX_LUN                0xFE
#define MASS_STORAGE_RESET         0xFF
#define LUN_DATA_LENGTH            1

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USB_APP_init(void);
void USB_APP_Reset(void);
void USB_APP_SetConfiguration(void);
void USB_APP_ClearFeature(void);
void USB_APP_SetDeviceAddress (void);
void USB_APP_Status_In (void);
void USB_APP_Status_Out (void);
RESULT USB_APP_Data_Setup(u8);
RESULT USB_APP_NoData_Setup(u8);
RESULT USB_APP_Get_Interface_Setting(u8 Interface, u8 AlternateSetting);
u8 *USB_APP_GetDeviceDescriptor(u16 );
u8 *USB_APP_GetConfigDescriptor(u16);
u8 *USB_APP_GetStringDescriptor(u16);

RESULT Keyboard_SetProtocol(void);
u8 *Keyboard_GetProtocolValue(u16 Length);
//RESULT Keyboard_SetProtocol(void);
u8 *Keyboard_GetReportDescriptor(u16 Length);
u8 *Keyboard_GetHIDDescriptor(u16 Length);
u8 *Keyboard_SetReport(u16 Length);

u8 *Virtual_Com_Port_GetLineCoding(u16 Length);
u8 *Virtual_Com_Port_SetLineCoding(u16 Length);

void USB_Set_Descriptor(void);
u8 *Get_Max_Lun(u16 Length);
#endif /* __USB_PROP_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
