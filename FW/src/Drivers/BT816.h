#ifndef _BT816_H_
#define _BT816_H_

#define		BT816_RES_BUFFER_LEN			256		//数据帧缓冲大小取决于蓝牙打印机APP拆包的大小
													//也就是说假如打印机需要通过SPP发送一个很大的数据包下来，应该会拆分成
													//小的数据包发送出来，每个蓝牙打印机APP的拆包机制可能不一样，所以此数据
													//在内存能够允许的情况下尽量开大一下，对打印APP的兼容性会好很多。

#define		SPP_BUFFER_LEN					1024	//这个是纯粹的串口环形缓冲区大小，由于缓冲区始终存在溢出的风险，而且目前貌似
													//没有数据同步机制，接收缓冲区溢出发生时无法通知打印APP，所以可能存在丢失数据的风险。
													//也只能


#define USE_BT1_MODULE		(1<<0)
#define USE_BT2_MODULE		(1<<1)
#define USE_BT3_MODULE		(1<<2)
#define USE_BT4_MODULE		(1<<3)

//此宏配置，根据具体的硬件实际使用那几个蓝牙模块来决定的
#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4个蓝牙模块都同时工作

#define		SUPPORT_BT(ch)		((BT_MODULE_CONFIG&(ch))?1:0)
#define		MAX_PT_CHANNEL		(SUPPORT_BT(USE_BT1_MODULE)+SUPPORT_BT(USE_BT2_MODULE)+SUPPORT_BT(USE_BT3_MODULE)+SUPPORT_BT(USE_BT4_MODULE))

#define		BT1_MODULE			0
#define		BT2_MODULE			(BT1_MODULE+SUPPORT_BT(USE_BT1_MODULE))
#define		BT3_MODULE			(BT2_MODULE+SUPPORT_BT(USE_BT2_MODULE))
#define		BT4_MODULE			(BT3_MODULE+SUPPORT_BT(USE_BT3_MODULE))
	



extern unsigned char	BT816_recbuffer[MAX_PT_CHANNEL][BT816_RES_BUFFER_LEN];
extern struct ringbuffer	spp_ringbuf[MAX_PT_CHANNEL];


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
int BT816_query_version(unsigned int bt_channel,unsigned char *ver_buffer);
int BT816_query_name(unsigned int bt_channel,unsigned char *name);
int BT816_set_name(unsigned int bt_channel,unsigned char *name);
int BT816_connect_status(unsigned int bt_channel);
int BT816_Channel1_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel2_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel3_RxISRHandler(unsigned char *res, unsigned int res_len);
int BT816_Channel4_RxISRHandler(unsigned char *res, unsigned int res_len);
#endif