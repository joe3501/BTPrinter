#ifndef _BT816_H_
#define _BT816_H_

#define		BT816_RES_BUFFER_LEN			256		//����֡�����Сȡ����������ӡ��APP����Ĵ�С
													//Ҳ����˵�����ӡ����Ҫͨ��SPP����һ���ܴ�����ݰ�������Ӧ�û��ֳ�
													//С�����ݰ����ͳ�����ÿ��������ӡ��APP�Ĳ�����ƿ��ܲ�һ�������Դ�����
													//���ڴ��ܹ����������¾�������һ�£��Դ�ӡAPP�ļ����Ի�úܶࡣ

#define		SPP_BUFFER_LEN					1024	//����Ǵ���Ĵ��ڻ��λ�������С�����ڻ�����ʼ�մ�������ķ��գ�����Ŀǰò��
													//û������ͬ�����ƣ����ջ������������ʱ�޷�֪ͨ��ӡAPP�����Կ��ܴ��ڶ�ʧ���ݵķ��ա�
													//Ҳֻ��

//#define HID_MODE
#define SPP_MODE


extern unsigned char	BT816_recbuffer[BT816_RES_BUFFER_LEN];
#ifdef SPP_MODE
extern struct ringbuffer	spp_ringbuf;
#endif

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

int BT816_Reset(void);
int BT816_init(void);
int BT816_query_version(unsigned char *ver_buffer);
int BT816_query_name(unsigned char *name);
int BT816_set_name(unsigned char *name);
int BT816_set_baudrate(BT816_BAUDRATE baudrate);
int BT816_enter_pair_mode(void);
int BT816_set_hid_trans_mode(BT_HID_TRANS_MODE mode);
int BT816_set_profile(BT_PROFILE mode);
int BT816_hid_status(void);
int BT816_spp_status(void);
int BT816_hid_set_delay(unsigned int	delay);
int BT816_hid_connect_last_host(void);
int BT816_hid_disconnect(void);
int BT816_toggle_ioskeypad(void);
int BT816_hid_send(unsigned char *str,unsigned int len);
int BT816_set_autocon(unsigned int	enable);

int BT816_RxISRHandler(unsigned char *res, unsigned int res_len);
void BT816_enter_sleep(void);
void BT816_wakeup(void);

int BT816_hid_send_test(void);		//for test
#endif