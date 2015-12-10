#ifndef _CONFIG_H_
#define _CONFIG_H_


//#define PT_CHANNEL_ISOLATION		//定义此宏表示各个打印通道是相互隔离的，比如每个打印通道设置的参数不会相互影响，否则会相互影响。
									//开启了此宏定义，内存消耗会加大，因为每个通道都会保存一份设置的打印参数。
//===================================
#define PT486
//#define PT487
//#define PT488
//#define PT48D

#define VERSION_MAJOR	1
#define VERSION_MINOR	0
#define VERSION_TEST    1

//#define LOW_5V_PRINT
#define HIGH_8V_PRINT

//#define CODEPAGE
//#define TP_VOLTAGE_SNS
#define TEMP_SNS_ENABLE
#define TM_T88II
#define CHINESE_FONT
#define GBK
//#define FONT_DOWNLOAD


#endif
