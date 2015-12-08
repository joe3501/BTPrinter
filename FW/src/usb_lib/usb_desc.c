/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_desc.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Descriptors for Joystick Mouse Demo
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_app_config.h"
#include "usb_lib.h"
#include "usb_desc.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* USB Standard Device Descriptor */
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
const u8 Virtual_Com_Port_DeviceDescriptor[] =
{
	0x12,   /* bLength */
	USB_DEVICE_DESCRIPTOR_TYPE,     /* bDescriptorType */
	0x00,
	0x02,   /* bcdUSB = 2.00 */
	0x02,   /* bDeviceClass: CDC */
	0x00,   /* bDeviceSubClass */
	0x00,   /* bDeviceProtocol */
	0x40,   /* bMaxPacketSize0 */
	0x83,
	0x04,   /* idVendor = 0x0483 */
	0x40,
	0x57,   /* idProduct = 0x5740 */
	0x00,
	0x02,   /* bcdDevice = 2.00 */
	1,              /* Index of string descriptor describing manufacturer */
	2,              /* Index of string descriptor describing product */
	3,              /* Index of string descriptor describing the device's serial number */
	0x01    /* bNumConfigurations */
};
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
const u8 Keyboard_DeviceDescriptor[USB_APP_SIZ_DEVICE_DESC] =
  {
    0x12,                       /*bLength */
    USB_DEVICE_DESCRIPTOR_TYPE, /*bDescriptorType*/
    0x00,                       /*bcdUSB */
    0x02,
    0x00,                       /*bDeviceClass*/
    0x00,                       /*bDeviceSubClass*/
    0x00,                       /*bDeviceProtocol*/
    0x40,                       /*bMaxPacketSize40*/
    0x80,                       /*idVendor (0x0483)*/
    0x04,
    0x10,                       /*idProduct = 0x5710*/
    0x60,
    0x00,                       /*bcdDevice rel. 2.00*/
    0x02,
    1,                          /*Index of string descriptor describing
                                                  manufacturer */
    2,                          /*Index of string descriptor describing
                                                 product*/
    3,                          /*Index of string descriptor describing the
                                                 device serial number */
    0x01                        /*bNumConfigurations*/
  }
  ; /* Joystick_DeviceDescriptor */
#endif

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor */
#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
  const u8 Virtual_Com_Port_ConfigDescriptor[] =
  {
	  /*Configuation Descriptor*/
	  0x09,   /* bLength: Configuation Descriptor size */
	  USB_CONFIGURATION_DESCRIPTOR_TYPE,      /* bDescriptorType: Configuration */
	  VIRTUAL_COM_PORT_SIZ_CONFIG_DESC,       /* wTotalLength:no of returned bytes */
	  0x00,
	  0x02,   /* bNumInterfaces: 2 interface */
	  0x01,   /* bConfigurationValue: Configuration value */
	  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
	  0xC0,   /* bmAttributes: self powered */
	  0x32,   /* MaxPower 0 mA */
	  /*Interface Descriptor*/
	  0x09,   /* bLength: Interface Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
	  /* Interface descriptor type */
	  0x00,   /* bInterfaceNumber: Number of Interface */
	  0x00,   /* bAlternateSetting: Alternate setting */
	  0x01,   /* bNumEndpoints: One endpoints used */
	  0x02,   /* bInterfaceClass: Communication Interface Class */
	  0x02,   /* bInterfaceSubClass: Abstract Control Model */
	  0x01,   /* bInterfaceProtocol: Common AT commands */
	  0x00,   /* iInterface: */
	  /*Header Functional Descriptor*/
	  0x05,   /* bLength: Endpoint Descriptor size */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x00,   /* bDescriptorSubtype: Header Func Desc */
	  0x10,   /* bcdCDC: spec release number */
	  0x01,
	  /*Call Managment Functional Descriptor*/
	  0x05,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
	  0x00,   /* bmCapabilities: D0+D1 */
	  0x01,   /* bDataInterface: 1 */
	  /*ACM Functional Descriptor*/
	  0x04,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
	  0x02,   /* bmCapabilities */
	  /*Union Functional Descriptor*/
	  0x05,   /* bFunctionLength */
	  0x24,   /* bDescriptorType: CS_INTERFACE */
	  0x06,   /* bDescriptorSubtype: Union func desc */
	  0x00,   /* bMasterInterface: Communication class interface */
	  0x01,   /* bSlaveInterface0: Data Class Interface */
	  /*Endpoint 2 Descriptor*/
	  0x07,   /* bLength: Endpoint Descriptor size */
	  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
	  0x82,   /* bEndpointAddress: (IN2) */
	  0x03,   /* bmAttributes: Interrupt */
	  VIRTUAL_COM_PORT_INT_SIZE,      /* wMaxPacketSize: */
	  0x00,
	  0xFF,   /* bInterval: */
	  /*Data class interface descriptor*/
	  0x09,   /* bLength: Endpoint Descriptor size */
	  USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
	  0x01,   /* bInterfaceNumber: Number of Interface */
	  0x00,   /* bAlternateSetting: Alternate setting */
	  0x02,   /* bNumEndpoints: Two endpoints used */
	  0x0A,   /* bInterfaceClass: CDC */
	  0x00,   /* bInterfaceSubClass: */
	  0x00,   /* bInterfaceProtocol: */
	  0x00,   /* iInterface: */
	  /*Endpoint 3 Descriptor*/
	  0x07,   /* bLength: Endpoint Descriptor size */
	  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
	  0x03,   /* bEndpointAddress: (OUT3) */
	  0x02,   /* bmAttributes: Bulk */
	  VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
	  0x00,
	  0x00,   /* bInterval: ignore for Bulk transfer */
	  /*Endpoint 1 Descriptor*/
	  0x07,   /* bLength: Endpoint Descriptor size */
	  USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
	  0x81,   /* bEndpointAddress: (IN1) */
	  0x02,   /* bmAttributes: Bulk */
	  VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
	  0x00,
	  0x00    /* bInterval */
  };
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_KEYBOARD_DEVICE)
const u8 Keyboard_ConfigDescriptor[KEYBOARD_SIZ_CONFIG_DESC] =
  {
    0x09, /* bLength: Configuation Descriptor size */
    USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType: Configuration */
    KEYBOARD_SIZ_CONFIG_DESC,
    /* wTotalLength: Bytes returned */
    0x00,
    0x01,         /*bNumInterfaces: 1 interface*/
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing
                                     the configuration*/
    0xE0,         /*bmAttributes: bus powered */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/

    /************** Descriptor of Joystick Mouse interface ****************/
    /* 09 */
    0x09,         /*bLength: Interface Descriptor size*/
    USB_INTERFACE_DESCRIPTOR_TYPE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0,            /*iInterface: Index of string descriptor*/
    /******************** Descriptor of Joystick Mouse HID ********************/
    /* 18 */
    0x09,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x00,         /*bcdHID: HID Class Spec release number*/
    0x01,
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    0x22,         /*bDescriptorType*/
    KEYBOARD_SIZ_REPORT_DESC,/*wItemLength: Total length of Report descriptor*/
    0x00,
    /******************** Descriptor of Joystick Mouse endpoint ********************/
    /* 27 */
    0x07,          /*bLength: Endpoint Descriptor size*/
    USB_ENDPOINT_DESCRIPTOR_TYPE, /*bDescriptorType:*/

    0x81,          /*bEndpointAddress: Endpoint Address (IN)*/
    0x03,          /*bmAttributes: Interrupt endpoint*/
    0x03,          /*wMaxPacketSize: 8 Byte max */
    0x00,
    0x02,          /*bInterval: Polling Interval (32 ms)*/
    /* 34 */
  }
  ; /* MOUSE_ConfigDescriptor */
const u8 Keyboard_ReportDescriptor[KEYBOARD_SIZ_REPORT_DESC] =
  {
#if 0
    0x05,  0x01,       /*Usage Page(Generic Desktop)*/
    0x09,  0x06,       /*Usage(keyboard)*/
    0xA1,  0x01,        /*Collection(Logical)*/
  
    0x05,  0x07,        /*Usage Page(Keyboard)*/
    0x19,  0x00,       /*Usage Minimum(1)*/
    0x29,  0xe7,         /*Usage Maximum(3)*/
    /* 16 */
    0x15,  0x00,        /*Logical Minimum(0)*/
    0x25,  0xFF,        /*Logical Maximum(1)*/
    0x95, 0x1,         /*Report Count(8)*/
    0x75, 0x08,         /*Report Size(8)*/
    /* 24 */
    0x81, 0x00,         /*Input(Variable)*/
	//关集合，跟上面的对应
	0xc0                           // END_COLLECTION
#endif

	//此报表描述符定义了一个输入报告和一个输出报告
	//其中书输入报告由3个字节组成第一个值表明是否有特殊键按下，第二个固定为0，第三个键表示按下的按键
	//输出报告由一个字节组成，其中前5个字节有效，分别控制键盘的LED的状态

	0x05,0x01,		/*Usage Page(Generic Desktop)*/
	0x09,0x06,		/*Usage(keyboard)*/

	0xa1,0x01,		/*Collection(Application)*/
	0x05,0x07,		 /*Usage Page(Keyboard)*/
	0x19,0xe0,		/*Usage Minimum(0)*/	//左CTRL
	0x29,0xe7,		/*Usage Maximum(1)*/	//右GUI键	
	0x15,0x00,		/*Logical Minimum(0)*/
	0x25,0x01,		/*Logical Maximum(1)*/
	0x95,0x08,		/*Report Count(8)*/
	0x75,0x01,		/*Report Size(1)*/
	0x81,0x02,		/*Input(DataVarAbs)*/

	0x95,0x01,		/*Report Count(1)*/
	0x75,0x08,		/*Report Size(8)*/
	0x81,0x03,		/*Input(CnstVarAbs)*/	//设备必须返回0

	0x95,0x05,		/*Report Count(5)*/
	0x75,0x01,		/*Report Size(1)*/
	0x05,0x08,		/*Usage Page(LEDs) */
	0x19,0x01,		/*Usage Minimum(0)*/	//NumberLock指示灯
	0x29,0x05,		/*Usage Maximum(5)*/	//kanna指示灯	
	0x91,0x02,		/*Onput(DataVarAbs)*/	//输出

	0x95,0x01,		/*Report Count(1)*/
	0x75,0x03,		/*Report Size(3)*/
	0x91,0x03,		/*Onput(CnstVarAbs)*/	//输出

	0x95,0x01,		/*Report Count(1)*/
	0x75,0x08,		/*Report Size(8)*/

	0x15,0x00,		/*Logical Minimum(0)*/
	0x25,0xff,		/*Logical Maximum(1)*/

	0x05,0x07,		 /*Usage Page(Keyboard)*/
	0x19,0x00,		/*Usage Minimum(0)*/
	0x29,0x65,		/*Usage Maximum(1)*/

	0x81,0x00,		/*Input(DataVarAbs)*/

	0xc0
}; /* Joystick_ReportDescriptor */

#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
const u8 MASS_DeviceDescriptor[MASS_SIZ_DEVICE_DESC] =
{
	0x12,   /* bLength  */
	0x01,   /* bDescriptorType */
	0x00,   /* bcdUSB, version 2.00 */
	0x02,
	0x00,   /* bDeviceClass : each interface define the device class */
	0x00,   /* bDeviceSubClass */
	0x00,   /* bDeviceProtocol */
	0x40,   /* bMaxPacketSize0 0x40 = 64 */
	0x83,   /* idVendor     (0483) */
	0x04,
	0x20,   /* idProduct */
	0x57,
	0x00,   /* bcdDevice 2.00*/
	0x02,
	1,              /* index of string Manufacturer  */
	/**/
	2,              /* index of string descriptor of product*/
	/* */
	3,              /* */
	/* */
	/* */
	0x01    /*bNumConfigurations */
};
const u8 MASS_ConfigDescriptor[MASS_SIZ_CONFIG_DESC] =
{

	0x09,   /* bLength: Configuation Descriptor size */
	0x02,   /* bDescriptorType: Configuration */
	MASS_SIZ_CONFIG_DESC,

	0x00,
	0x01,   /* bNumInterfaces: 1 interface */
	0x01,   /* bConfigurationValue: */
	/*      Configuration value */
	0x00,   /* iConfiguration: */
	/*      Index of string descriptor */
	/*      describing the configuration */
	0xC0,   /* bmAttributes: */
	/*      bus powered */
	0x32,   /* MaxPower 100 mA */

	/******************** Descriptor of Mass Storage interface ********************/
	/* 09 */
	0x09,   /* bLength: Interface Descriptor size */
	0x04,   /* bDescriptorType: */
	/*      Interface descriptor type */
	0x00,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x02,   /* bNumEndpoints*/
	0x08,   /* bInterfaceClass: MASS STORAGE Class */
	0x06,   /* bInterfaceSubClass : SCSI transparent*/
	0x50,   /* nInterfaceProtocol */
	4,          /* iInterface: */
	/* 18 */
	0x07,   /*Endpoint descriptor length = 7*/
	0x05,   /*Endpoint descriptor type */
	0x81,   /*Endpoint address (IN, address 1) */
	0x02,   /*Bulk endpoint type */
	0x40,   /*Maximum packet size (64 bytes) */
	0x00,
	//0x40,
	//0x00,	//   joe 修改
	0x00,   /*Polling interval in milliseconds */
	/* 25 */
	0x07,   /*Endpoint descriptor length = 7 */
	0x05,   /*Endpoint descriptor type */
	0x02,   /*Endpoint address (OUT, address 2) */
	0x02,   /*Bulk endpoint type */
	0x40,   /*Maximum packet size (64 bytes) */
	0x00,
	//0x40,
	//0x00,	//   joe 修改
	0x00     /*Polling interval in milliseconds*/
	/*32*/
};
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
const u8 Printer_DeviceDescriptor[PRINTER_SIZ_DEVICE_DESC] =
{
	0x12,   /* bLength  */
	0x01,   /* bDescriptorType */
	0x00,   /* bcdUSB, version 2.00 */
	0x02,
	0x00,   /* bDeviceClass : each interface define the device class */
	0x00,   /* bDeviceSubClass */
	0x00,   /* bDeviceProtocol */
	0x40,   /* bMaxPacketSize0 0x40 = 64 */
	0x00,   /* idVendor     (0483) */
	0xb0,
	0x11,   /* idProduct */
	0x04,
	0x00,   /* bcdDevice 2.00*/
	0x02,
	1,              /* index of string Manufacturer  */
	/**/
	2,              /* index of string descriptor of product*/
	/* */
	3,              /* */
	/* */
	/* */
	0x01    /*bNumConfigurations */
};
const u8 Printer_ConfigDescriptor[PRINTER_SIZ_CONFIG_DESC] =
{
#if 1
	0x09,   /* bLength: Configuation Descriptor size */
	0x02,   /* bDescriptorType: Configuration */
	PRINTER_SIZ_CONFIG_DESC,

	0x00,
	0x01,   /* bNumInterfaces: 1 interface */
	0x01,   /* bConfigurationValue: */
	/*      Configuration value */
	0x00,   /* iConfiguration: */
	/*      Index of string descriptor */
	/*      describing the configuration */
	0xC0,   /* bmAttributes: */
	/*      bus powered */
	0x32,   /* MaxPower 100 mA */

	/******************** Descriptor of Printer interface ********************/
	/* 09 */
	0x09,   /* bLength: Interface Descriptor size */
	0x04,   /* bDescriptorType: */
	/*      Interface descriptor type */
	0x00,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x02,   /* bNumEndpoints*/
	0x07,   /* bInterfaceClass: Printer Class */
	0x01,   /* bInterfaceSubClass : printer*/
	0x02,   /* nInterfaceProtocol :  01: Unidirectional interface  02：Bi-directional interface  03：1284.4 compatible bi-directional interface*/
	4,          /* iInterface: */
	/* 18 */
	0x07,   /*Endpoint descriptor length = 7*/
	0x05,   /*Endpoint descriptor type */
	0x81,   /*Endpoint address (IN, address 1) */
	0x02,   /*Bulk endpoint type */
	0x40,   /*Maximum packet size (64 bytes) */
	0x00,
	0x00,   /*Polling interval in milliseconds */
	/* 25 */
	0x07,   /*Endpoint descriptor length = 7 */
	0x05,   /*Endpoint descriptor type */
	0x02,   /*Endpoint address (OUT, address 2) */
	0x02,   /*Bulk endpoint type */
	0x40,   /*Maximum packet size (64 bytes) */
	0x00,
	0x00     /*Polling interval in milliseconds*/
#endif

#if 0
	0x09,   /* bLength: Configuation Descriptor size */
	0x02,   /* bDescriptorType: Configuration */
	PRINTER_SIZ_CONFIG_DESC,

	0x00,
	0x02,   /* bNumInterfaces: 1 interface */
	0x01,   /* bConfigurationValue: */
	/*      Configuration value */
	0x00,   /* iConfiguration: */
	/*      Index of string descriptor */
	/*      describing the configuration */
	0xC0,   /* bmAttributes: */
	/*      bus powered */
	0x32,   /* MaxPower 100 mA */

	/******************** Descriptor of Printer interface ********************/
	/* 09 */
	0x09,   /* bLength: Interface Descriptor size */
	0x04,   /* bDescriptorType: */
	/*      Interface descriptor type */
	0x00,   /* bInterfaceNumber: Number of Interface */
	0x00,   /* bAlternateSetting: Alternate setting */
	0x01,   /* bNumEndpoints*/
	0x07,   /* bInterfaceClass: Printer Class */
	0x01,   /* bInterfaceSubClass : printer*/
	0x02,   /* nInterfaceProtocol :  01: Unidirectional interface  02：Bi-directional interface  03：1284.4 compatible bi-directional interface*/
	0,          /* iInterface: */
	/* 18 */
	0x07,   /*Endpoint descriptor length = 7*/
	0x05,   /*Endpoint descriptor type */
	0x02,   /*Endpoint address (IN, address 1) */
	0x02,   /*Bulk endpoint type */
	0x40,   /*Maximum packet size (64 bytes) */
	0x00,
	0x00,   /*Polling interval in milliseconds */
	0x09,
	0x04,
	0x01,
	0x00,
	0x02,
	0x03,
	0x00,
	0x00,
	0x03,
	0x09,
	0x21,
	0x10,
	0x01,
	0x00,
	0x01,
	0x22,
	0x1b,
	0x00,
	0x07,
	0x05,
	0x82,
	0x03,
	0x40,
	0x00,
	0x01,
	0x07,
	0x05,
	0x03,
	0x03,
	0x40,
	0x00,
	0x01
#endif
};
#endif

/* USB String Descriptors (optional) */
const u8 USB_APP_StringLangID[USB_APP_SIZ_STRING_LANGID] =
  {
    USB_APP_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04
  }
  ; /* LangID = 0x0409: U.S. English */

const u8 USB_APP_StringVendor[USB_APP_SIZ_STRING_VENDOR] =
  {
    USB_APP_SIZ_STRING_VENDOR, /* Size of Vendor string */
    USB_STRING_DESCRIPTOR_TYPE,  /* bDescriptorType*/
    /* Manufacturer: "STMicroelectronics" */
    'H', 0, 'E', 0, 'R', 0, 'O', 0, 'J', 0, 'E', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0, ' ', 0,
    ' ', 0, ' ', 0
  };

const u8 USB_APP_StringProduct[USB_APP_SIZ_STRING_PRODUCT] =
  {
    USB_APP_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'H', 0, 'J', 0, ' ', 0, 'U', 0, 'S', 0, 'B', 0, 'P', 0,
    'r', 0, 'i', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, ' ', 0
  };

u8 USB_APP_StringSerial[USB_APP_SIZ_STRING_SERIAL] =
  {
    USB_APP_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    'K', 0, 'T', 0, '4', 0, '8', 0, '6', 0, '1', 0, '0', 0
  };

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

