#ifndef _BT816_H_
#define _BT816_H_

#define		BT816_RES_BUFFER_LEN			512		//数据帧缓冲大小取决于蓝牙打印机APP拆包的大小
													//也就是说假如打印机需要通过SPP发送一个很大的数据包下来，应该会拆分成
													//小的数据包发送出来，每个蓝牙打印机APP的拆包机制可能不一样，所以此数据
													//在内存能够允许的情况下尽量开大一下，对打印APP的兼容性会好很多。

													//测试的时候发现会丢数据，就是因为这个DMA缓冲区开小了，导致蓝牙模块传输上来的数据丢掉了一部分。
													//蓝牙因为流控被阻塞时，会将主机发送的赌赛在其内部的数据一次传输过来。测试的时候发现其一次最多传了384字节
													//

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


#define RING_BUFF_FULL_TH		(SPP_BUFFER_LEN-256)	//当ringbuffer中接收到的数据大于此值时，实行流控，通知蓝牙模块不要再传数据下来了，此值待调试确定
#define RING_BUFF_EMPTY_TH		(SPP_BUFFER_LEN/2)	//当ringbuffer中接收到的数据大于此值时，实行流控，通知蓝牙模块不要再传数据下来了，此值待调试确定

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