/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_desc.h
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Descriptor Header for Joystick Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define KEYBOARD_SIZ_HID_DESC                   0x09
#define KEYBOARD_OFF_HID_DESC                   0x12

#define USB_APP_SIZ_DEVICE_DESC                18
#define USB_APP_SIZ_STRING_LANGID              4
#define USB_APP_SIZ_STRING_VENDOR              38
#define USB_APP_SIZ_STRING_SERIAL              26
#define USB_APP_SIZ_STRING_PRODUCT             30

#define KEYBOARD_SIZ_CONFIG_DESC                34
#define KEYBOARD_SIZ_REPORT_DESC				63//23



#define STANDARD_ENDPOINT_DESC_SIZE             0x09

#define VIRTUAL_COM_PORT_DATA_SIZE              64
#define VIRTUAL_COM_PORT_INT_SIZE               8
#define VIRTUAL_COM_PORT_SIZ_CONFIG_DESC        67
//#define VIRTUAL_COM_PORT_SIZ_STRING_PRODUCT     50

#define MASS_SIZ_DEVICE_DESC              18
#define MASS_SIZ_CONFIG_DESC              32

#define MASS_SIZ_STRING_LANGID            4
#define MASS_SIZ_STRING_VENDOR            38
#define MASS_SIZ_STRING_PRODUCT           38
#define MASS_SIZ_STRING_SERIAL            26
#define MASS_SIZ_STRING_INTERFACE         16

/* Exported functions ------------------------------------------------------- */
extern const u8 MASS_DeviceDescriptor[MASS_SIZ_DEVICE_DESC];
extern const u8 MASS_ConfigDescriptor[MASS_SIZ_CONFIG_DESC];

extern const u8 MASS_StringLangID[MASS_SIZ_STRING_LANGID];
extern const u8 MASS_StringVendor[MASS_SIZ_STRING_VENDOR];
extern const u8 MASS_StringProduct[MASS_SIZ_STRING_PRODUCT];
extern u8 MASS_StringSerial[MASS_SIZ_STRING_SERIAL];
extern const u8 MASS_StringInterface[MASS_SIZ_STRING_INTERFACE];


/* Exported functions ------------------------------------------------------- */
extern const u8 Keyboard_DeviceDescriptor[USB_APP_SIZ_DEVICE_DESC];
extern const u8 Keyboard_ConfigDescriptor[KEYBOARD_SIZ_CONFIG_DESC];
extern const u8 Keyboard_ReportDescriptor[KEYBOARD_SIZ_REPORT_DESC];
//extern const u8 Keyboard_StringLangID[USB_APP_SIZ_STRING_LANGID];
//extern const u8 Keyboard_StringVendor[USB_APP_SIZ_STRING_VENDOR];
//extern const u8 Keyboard_StringProduct[KEYBOARD_SIZ_STRING_PRODUCT];
//extern u8 Keyboard_StringSerial[USB_APP_SIZ_STRING_SERIAL];

extern const u8 Virtual_Com_Port_DeviceDescriptor[USB_APP_SIZ_DEVICE_DESC];
extern const u8 Virtual_Com_Port_ConfigDescriptor[VIRTUAL_COM_PORT_SIZ_CONFIG_DESC];

extern u8 USB_APP_StringSerial[USB_APP_SIZ_STRING_SERIAL];
extern const u8 USB_APP_StringLangID[USB_APP_SIZ_STRING_LANGID];
extern const u8 USB_APP_StringVendor[USB_APP_SIZ_STRING_VENDOR];
extern const u8 USB_APP_StringProduct[USB_APP_SIZ_STRING_PRODUCT];

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
