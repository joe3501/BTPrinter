#ifndef _BT816_H_
#define _BT816_H_

#define		BT816_RES_BUFFER_LEN			512		//����֡�����Сȡ����������ӡ��APP����Ĵ�С
													//Ҳ����˵�����ӡ����Ҫͨ��SPP����һ���ܴ�����ݰ�������Ӧ�û��ֳ�
													//С�����ݰ����ͳ�����ÿ��������ӡ��APP�Ĳ�����ƿ��ܲ�һ�������Դ�����
													//���ڴ��ܹ����������¾�������һ�£��Դ�ӡAPP�ļ����Ի�úܶࡣ

													//���Ե�ʱ���ֻᶪ���ݣ�������Ϊ���DMA��������С�ˣ���������ģ�鴫�����������ݶ�����һ���֡�
													//������Ϊ���ر�����ʱ���Ὣ�������͵Ķ��������ڲ�������һ�δ�����������Ե�ʱ������һ����ഫ��384�ֽ�
													//

#define		SPP_BUFFER_LEN					1024	//����Ǵ���Ĵ��ڻ��λ�������С�����ڻ�����ʼ�մ�������ķ��գ�����Ŀǰò��
													//û������ͬ�����ƣ����ջ������������ʱ�޷�֪ͨ��ӡAPP�����Կ��ܴ��ڶ�ʧ���ݵķ��ա�
													//Ҳֻ��


#define USE_BT1_MODULE		(1<<0)
#define USE_BT2_MODULE		(1<<1)
#define USE_BT3_MODULE		(1<<2)
#define USE_BT4_MODULE		(1<<3)

//�˺����ã����ݾ����Ӳ��ʵ��ʹ���Ǽ�������ģ����������
#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4������ģ�鶼ͬʱ����

#define		SUPPORT_BT(ch)		((BT_MODULE_CONFIG&(ch))?1:0)
#define		MAX_BT_CHANNEL		(SUPPORT_BT(USE_BT1_MODULE)+SUPPORT_BT(USE_BT2_MODULE)+SUPPORT_BT(USE_BT3_MODULE)+SUPPORT_BT(USE_BT4_MODULE))

#define		BT1_MODULE			0
#define		BT2_MODULE			(BT1_MODULE+SUPPORT_BT(USE_BT1_MODULE))
#define		BT3_MODULE			(BT2_MODULE+SUPPORT_BT(USE_BT2_MODULE))
#define		BT4_MODULE			(BT3_MODULE+SUPPORT_BT(USE_BT3_MODULE))
	
#define		trip1()	do{\
	GPIO_ResetBits(GPIOB, GPIO_Pin_6);\
	GPIO_SetBits(GPIOB, GPIO_Pin_6);\
}while(0)

#define		trip2()	do{\
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);\
	GPIO_SetBits(GPIOB, GPIO_Pin_5);\
}while(0)

#define		trip3()	do{\
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);\
	GPIO_SetBits(GPIOB, GPIO_Pin_4);\
}while(0)

#define		trip4()	do{\
	GPIO_ResetBits(GPIOB, GPIO_Pin_3);\
	GPIO_SetBits(GPIOB, GPIO_Pin_3);\
}while(0)

extern unsigned char	BT816_recbuffer[][BT816_RES_BUFFER_LEN];


//��������ģ��֧�ֵĲ�����
//@note  δ����֤��ģ������Ҳû�и�����֧�ֵĲ������б�
typedef enum
{
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_43000,
	BAUDRATE_57600,
	BAUDRATE_115200
}BT816_BAUDRATE;


//��������ģ��֧�ֵ�profile
typedef enum
{
	BT_PROFILE_HID,
	BT_PROFILE_SPP,
	BT_PROFILE_BLE
}BT_PROFILE;

//��������ģ��HID��ֵ����ģʽ
typedef enum
{
	BT_HID_TRANS_MODE_TRANSP,		//͸��
	BT_HID_TRANS_MODE_AT			//����
}BT_HID_TRANS_MODE;

#define BT_MODULE_STATUS_CONNECTED		1
#define BT_MODULE_STATUS_DISCONNECT		2


#define RING_BUFF_FULL_TH		(SPP_BUFFER_LEN-256)	//��ringbuffer�н��յ������ݴ��ڴ�ֵʱ��ʵ�����أ�֪ͨ����ģ�鲻Ҫ�ٴ����������ˣ���ֵ������ȷ��
#define RING_BUFF_EMPTY_TH		(SPP_BUFFER_LEN/2)	//��ringbuffer�н��յ������ݴ��ڴ�ֵʱ��ʵ�����أ�֪ͨ����ģ�鲻Ҫ�ٴ����������ˣ���ֵ������ȷ��

#define     set_BT1_BUSY()	do{\
	GPIO_SetBits(GPIOB, GPIO_Pin_8);\
	}while(0)

#define     set_BT1_FREE()	do{\
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);\
	}while(0)

#define     set_BT2_BUSY()	GPIO_SetBits(GPIOC, GPIO_Pin_1)
#define     set_BT2_FREE()	GPIO_ResetBits(GPIOC, GPIO_Pin_1)

#define     set_BT3_BUSY()	GPIO_SetBits(GPIOE, GPIO_Pin_15)
#define     set_BT3_FREE()	GPIO_ResetBits(GPIOE, GPIO_Pin_15)

#define     set_BT4_BUSY()	GPIO_SetBits(GPIOD, GPIO_Pin_0)
#define     set_BT4_FREE()	GPIO_ResetBits(GPIOD, GPIO_Pin_0)

#define     set_BT_FREE(ch)	do{\
	switch(ch)\
	{\
	case BT1_MODULE:\
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);\
		break;\
	case BT2_MODULE:\
		GPIO_ResetBits(GPIOC, GPIO_Pin_1);\
		break;\
	case BT3_MODULE:\
		GPIO_ResetBits(GPIOE, GPIO_Pin_15);\
		break;\
	case BT4_MODULE:\
		GPIO_ResetBits(GPIOD, GPIO_Pin_0);\
		break;\
	default:\
		break;\
	}\
}while(0)


int BT816_Reset(void);
int BT816_init(void);
int BT816_query_version(unsigned int bt_channel,unsigned char *ver_buffer);
int BT816_query_name(unsigned int bt_channel,unsigned char *name);
int BT816_set_name(unsigned int bt_channel,unsigned char *name);
int BT816_set_pin(unsigned int bt_channel,unsigned char *pin);
int BT816_connect_status(unsigned int bt_channel);
void BT816_send_data(unsigned int bt_channel,unsigned char *data,unsigned int len);
int BT816_Channel1_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel2_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel3_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel4_RxISRHandler(unsigned char *res, unsigned int res_len);
#endif