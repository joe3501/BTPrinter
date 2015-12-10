#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

//蓝牙模块类型
#define		USE_WBTDS01		1
#define		USE_BT816		2

//定义使用的蓝牙模块
#define BT_MODULE		USE_BT816

#define SMALL_MEMORY_CFG			//开启小内存模式，开启此宏是为了尽量节省内存，将整个项目的内存空间尽量压缩到20K

#ifdef SMALL_MEMORY_CFG
#define SOFT_TIMER_FUNC_ENABLE			0
#else
#define SOFT_TIMER_FUNC_ENABLE			1
#endif

#include "spi_flash.h"
#include "data_uart.h"
#include "BT816.h"
#include "Esc_p.h"
#include "print_head.h"
#include "PaperDetect.h"
#include  "KeyScan.h"
#include "basic_fun.h"

#endif
