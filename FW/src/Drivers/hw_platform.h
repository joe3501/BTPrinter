#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

//蓝牙模块类型
#define		USE_WBTDS01		1
#define		USE_BT816		2

//定义使用的蓝牙模块
#define BT_MODULE		USE_BT816

#include "spi_flash.h"
#include "data_uart.h"
#include "BT816.h"
#include "Esc_p.h"
#include "print_head.h"
#include "PaperDetect.h"
#include  "KeyScan.h"

#endif
