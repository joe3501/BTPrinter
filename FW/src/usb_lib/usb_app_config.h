#ifndef _USB_APP_CONFIG_H_
#define _USB_APP_CONFIG_H_
#include "usb_lib.h"

#define	 _USE_USB_KEYBOARD_DEVICE		(1<<0)
#define	 _USE_USB_VIRTUAL_COMM_DEVICE	(1<<1)
#define	 _USE_USB_MASS_STOARGE_DEVICE	(1<<2)


//����USB��������
#define USB_DEVICE_NONE		0	//��û��ʵ��USB�豸��״̬
#define USB_VIRTUAL_PORT	1	//���⴮��
#define USB_KEYBOARD		2	//USB����
#define USB_MASSSTORAGE		3	//�������洢�豸

//����USB_MASS_STOARGE_DEVICE������
#define MASSTORAGE_DEVICE_TYPE_DUMMY_FAT		0	//�����ļ�ϵͳ�豸
#define MASSTORAGE_DEVICE_TYPE_MSD				1	//MicroSD��
#define MASSTORAGE_DEVICE_TYPE_SPI_FLASH		2	//SPI_FLASH


/////////////////////////////////////////�û����õĺ����////////////////////////////////////////////////////////
//////////////////////////ע�������Ͽ���ȫ��������USB�豸��������Ӧ�ò���Ҫʹ��ȫ��USB�豸������£�ֻ����Ӧ��
//////////////////////////���ÿ��Խ�ʡ����ռ�////////////////////////////////////////////////////////////////////
//Ӧ������Ҫ�õ�ĳ��USB�豸���� | �ϼ���
//#define		USB_DEVICE_CONFIG		(_USE_USB_KEYBOARD_DEVICE)
//#define		USB_DEVICE_CONFIG		(_USE_USB_VIRTUAL_COMM_DEVICE)
#define		USB_DEVICE_CONFIG		(_USE_USB_MASS_STOARGE_DEVICE)
//#define		USB_DEVICE_CONFIG		(_USE_USB_VIRTUAL_COMM_DEVICE | _USE_USB_KEYBOARD_DEVICE | _USE_USB_MASS_STOARGE_DEVICE)//Ӧ����ֻ��ʹ�õ�USB�����豸��USB�������洢�豸
//#define	DUMMY_FAT_FS		//�����Ҫ���������ļ�ϵͳ����Ҫ�򿪴˺�
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