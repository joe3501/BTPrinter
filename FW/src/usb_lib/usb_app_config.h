#ifndef _USB_APP_CONFIG_H_
#define _USB_APP_CONFIG_H_
#include "usb_lib.h"

#define	 _USE_USB_KEYBOARD_DEVICE		(1<<0)
#define	 _USE_USB_VIRTUAL_COMM_DEVICE	(1<<1)
#define	 _USE_USB_MASS_STOARGE_DEVICE	(1<<2)


//定义USB具体类型
#define USB_DEVICE_NONE		0	//还没有实现USB设备的状态
#define USB_VIRTUAL_PORT	1	//虚拟串口
#define USB_KEYBOARD		2	//USB键盘
#define USB_MASSSTORAGE		3	//大容量存储设备

//定义USB_MASS_STOARGE_DEVICE的子类
#define MASSTORAGE_DEVICE_TYPE_DUMMY_FAT		0	//虚拟文件系统设备
#define MASSTORAGE_DEVICE_TYPE_MSD				1	//MicroSD卡
#define MASSTORAGE_DEVICE_TYPE_SPI_FLASH		2	//SPI_FLASH


/////////////////////////////////////////用户配置的宏参数////////////////////////////////////////////////////////
//////////////////////////注：理论上可以全部打开三种USB设备，但是在应用不需要使用全部USB设备的情况下，只打开相应的
//////////////////////////配置可以节省代码空间////////////////////////////////////////////////////////////////////
//应用中需要用到某种USB设备，请 | 上即可
//#define		USB_DEVICE_CONFIG		(_USE_USB_KEYBOARD_DEVICE)
//#define		USB_DEVICE_CONFIG		(_USE_USB_VIRTUAL_COMM_DEVICE)
#define		USB_DEVICE_CONFIG		(_USE_USB_MASS_STOARGE_DEVICE)
//#define		USB_DEVICE_CONFIG		(_USE_USB_VIRTUAL_COMM_DEVICE | _USE_USB_KEYBOARD_DEVICE | _USE_USB_MASS_STOARGE_DEVICE)//应用中只会使用到USB键盘设备和USB大容量存储设备
//#define	DUMMY_FAT_FS		//如果需要开启虚拟文件系统，需要打开此宏
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define		SUPPORT_USB_TYPE(c)		((USB_DEVICE_CONFIG & (c))?1:0)
#define		USB_DEVICE_TYPE_NUM		(SUPPORT_USB_TYPE(_USE_USB_KEYBOARD_DEVICE)+SUPPORT_USB_TYPE(_USE_USB_MASS_STOARGE_DEVICE)+SUPPORT_USB_TYPE(_USE_USB_VIRTUAL_COMM_DEVICE))

#define		VIRTUAL_COMM_DESC_OFFSET			0
#define		KEYBOARD_DESC_OFFSET			(VIRTUAL_COMM_DESC_OFFSET+SUPPORT_USB_TYPE(_USE_USB_VIRTUAL_COMM_DEVICE))
#define		MASS_DESC_OFFSET				(KEYBOARD_DESC_OFFSET+SUPPORT_USB_TYPE(_USE_USB_KEYBOARD_DEVICE))

#if(USB_DEVICE_CONFIG & _USE_USB_VIRTUAL_COMM_DEVICE)
extern unsigned char buffer_out[];
extern unsigned int	count_out;
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
extern unsigned char	g_mass_storage_device_type;
#ifdef DUMMY_FAT_FS
extern unsigned char	g_send_buff[];
#endif
#endif

extern unsigned char				g_usb_type;

void usb_Cable_Config (unsigned char NewState);
void usb_device_init(unsigned char device_type);
void usb_SendData(unsigned char *pData, int length);
void usb_Get_SerialNum(void);
#endif //PCUSART_H_