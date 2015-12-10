#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

//����ģ������
#define		USE_WBTDS01		1
#define		USE_BT816		2

//����ʹ�õ�����ģ��
#define BT_MODULE		USE_BT816

#define SMALL_MEMORY_CFG			//����С�ڴ�ģʽ�������˺���Ϊ�˾�����ʡ�ڴ棬��������Ŀ���ڴ�ռ価��ѹ����20K

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
