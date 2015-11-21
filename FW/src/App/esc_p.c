#include "hw_platform.h"
#include "ringbuffer.h"
#include <string.h>
#include "basic_fun.h"
#include "Terminal_Para.h"
#include "DotFill.h"
#include <assert.h>
#include "Font.h"
#include "uart.h"
//======================================================================================================
//======================================================================================================
//======================================================================================================
//#define NULL	(0x00)
#define SOH		(0x01)
#define STX		(0x02)
#define ETX		(0x03)
#define EOT		(0x04)
#define ENQ		(0x05)
#define ACK		(0x06)
#define BEL		(0x07)
#define BS		(0x08)
#define HT		(0x09)
#define LF		(0x0a)
#define VT		(0x0b)
#define FF		(0x0c)
#define CR		(0x0d)
#define SO		(0x0e)
#define SI		(0x0f)
#define DLE		(0x10)
#define DC1		(0x11)
#define DC2		(0x12)
#define DC3		(0x13)
#define DC4		(0x14)
#define NAK		(0x15)
#define SYN		(0x16)
#define ETB		(0x17)
#define ESC_CAN		(0x18)
#define EM		(0x19)
#define SUB		(0x1a)
#define ESC		(0x1b)
#define FS		(0x1c)
#define GS		(0x1d)
#define RS		(0x1e)
#define US		(0x1f)
#define SP		(0x20)

ESC_P_STS_T  esc_sts[MAX_PT_CHANNEL];
unsigned char	 current_channel;		//当前正在处理的通道
	
extern void esc_p_init(void)
{
	uint8_t i,n;
//----chang
	for (n = 0; n < MAX_PT_CHANNEL; n++)
	{
		esc_sts[n].international_character_set = 0;    // english
		esc_sts[n].character_code_page = g_param.character_code_page;

		esc_sts[n].prt_on = 0;
		esc_sts[n].larger = 0;
#ifdef ASCII9X24
		esc_sts[n].font_en = FONT_B_WIDTH;	// 字体
#else
		esc_sts[n].font_en = FONT_A_WIDTH;	// 字体
#endif
		esc_sts[n].font_cn = FONT_CN_A_WIDTH;	// 字体
		esc_sts[n].bold = 0;		// 粗体
		esc_sts[n].double_strike=0;//重叠打印
		esc_sts[n].underline = 0;	// 下划线
		esc_sts[n].revert = 0;		// 反白显示
		esc_sts[n].rotate = 0;
		esc_sts[n].start_dot = 0;
		esc_sts[n].smoothing_mode = 0;	// 平滑模式
		esc_sts[n].dot_minrow = ARRAY_SIZE(esc_sts[n].dot[0]);
		memset(esc_sts[n].dot, 0 ,sizeof(esc_sts[n].dot));
		for(i=0; i<8; i++)
		{
			esc_sts[n].tab[i] = 9+8*i;
		}
		esc_sts[n].linespace = 30;
		esc_sts[n].charspace = 0;
		esc_sts[n].align = 0;
		esc_sts[n].leftspace = 0;
		esc_sts[n].print_width=LineDot;
		esc_sts[n].upside_down=0;//倒置
		esc_sts[n].barcode_height = 50;
		esc_sts[n].barcode_width = 2;
		esc_sts[n].barcode_leftspace = 0;
		esc_sts[n].barcode_char_pos = 0;//不显示
		esc_sts[n].barcode_font = 0;
		esc_sts[n].userdefine_char = 0;
		esc_sts[n].asb_mode=0;

		esc_sts[n].chinese_mode = 1;
		esc_sts[n].bitmap_flag = 0;

		if(esc_sts[n].status4 == 0)
		{
			esc_sts[n].status4=0x12;
		}
	}

	current_channel = 0;
}
extern esc_init(void)
{
	esc_p_init();
}

extern void esc_p(void)
{
	uint8_t cmd;

	switch(cmd=Getchar())
	{
		case LF:	// line feed
			PrintCurrentBuffer(0);
			break;
		case CR:      // carry return
			break;
		case ESC:		// ESC
			break;
		case FS:		// FS
			break;
		case GS:		// GS
			break;
		case ESC_CAN:
			break;
		default:
			{
				//----chang
#if !defined(CHINESE_FONT)||defined (CODEPAGE)
				if((cmd >= 0x20) && (cmd <= 0xff))
				{
					GetEnglishFont(cmd);
				}
#else
				if((cmd >= 0x20) && (cmd <= 0x7f))
				{
					GetEnglishFont(cmd);
				}
#if defined(GBK) || defined(GB18030)
				else if ((cmd >= 0x81) && (cmd <= 0xfe))
				{
					uint8_t chs[4];
					chs[0] = cmd;
					chs[1] = Getchar();
#if defined(GB18030)
					if ((chs[1] >= 0x30) && (chs[1] <= 0x39))
#else
					if (0)
#endif
					{
						chs[2] = Getchar();
						chs[3] = Getchar();
						// GB18030定义的4字节扩展
						if (((chs[2] >= 0x81) && (chs[2] <= 0xfe)) && ((chs[3] >= 0x30) && (chs[3] <= 0x39)))
						{
							GetChineseFont(chs, CHINESE_FONT_GB18030);
						}
						else
						{
							GetEnglishFont('?');
							GetEnglishFont('?');
							GetEnglishFont('?');
							GetEnglishFont('?');
						}
					}
					// GB13000定义的2字节扩展
					else if ((chs[1] >= 0x40) && (chs[1] <= 0xfe) && (chs[1] != 0x7f))
					{
						GetChineseFont(chs, CHINESE_FONT_GB13000);
					}
					else
					{
						GetEnglishFont('?');
						GetEnglishFont('?');
					}
				}
#endif
#endif
			}

		}
}
//======================================================================================================

