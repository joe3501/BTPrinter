#ifndef _BT816_H_
#define _BT816_H_

#define		BT816_RES_BUFFER_LEN			256		//数据帧缓冲大小取决于蓝牙打印机APP拆包的大小
													//也就是说假如打印机需要通过SPP发送一个很大的数据包下来，应该会拆分成
													//小的数据包发送出来，每个蓝牙打印机APP的拆包机制可能不一样，所以此数据
													//在内存能够允许的情况下尽量开大一下，对打印APP的兼容性会好很多。

#define		SPP_BUFFER_LEN					1024	//这个是纯粹的串口环形缓冲区大小，由于缓冲区始终存在溢出的风险，而且目前貌似
													//没有数据同步机制，接收缓冲区溢出发生时无法通知打印APP，所以可能存在丢失数据的风险。
													//也只能

//#define HID_MODE
#define SPP_MODE


extern unsigned char	BT816_recbuffer[BT816_RES_BUFFER_LEN];
#ifdef SPP_MODE
extern struct ringbuffer	spp_ringbuf;
#endif

//定义蓝牙模块支持的波特率
//@note  未经验证，模块资料也没有给出其支持的波特率列表
typedef enum
{
	BAUDRATE_9600,
	BAUDRATE_19200,
	BAUDRATE_38400,
	BAUDRATE_43000,
	BAUDRATE_57600,
	BAUDRATE_115200
}BT816_BAUDRATE;


//定义蓝牙模块支持的profile
typedef enum
{
	BT_PROFILE_HID,
	BT_PROFILE_SPP,
	BT_PROFILE_BLE
}BT_PROFILE;

//定义蓝牙模块HID键值传输模式
typedef enum
{
	BT_HID_TRANS_MODE_TRANSP,		//透传
	BT_HID_TRANS_MODE_AT			//命令
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