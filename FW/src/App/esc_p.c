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
signed char	 current_channel;		//当前正在处理的通道

extern void esc_p_init(unsigned int n)
{
	uint8_t i;
	//----chang

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
	esc_sts[n].italic = 0;		// 斜体
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
extern esc_init(void)
{
	int i;
	for (i = 0; i < MAX_PT_CHANNEL; i++)
	{
		esc_p_init(i);
	}
	current_channel = -1;
}

extern void esc_p(void)
{
	uint8_t cmd;
	uint16_t  i,cnt,off;
	uint8_t chs[25],n;
        unsigned char tmp[LineDot/8];

	switch(cmd=Getchar())
	{
	case LF:	// line feed
		PrintCurrentBuffer(0);
		break;
	case CR:      // carry return
		//PrintCurrentBuffer(0);
		break;
#if 1
	case ESC:		// ESC
		chs[0] = Getchar();
		switch (chs[0])
		{
		case SP:
			chs[1] = Getchar();
			//ESC SP n 设置字符右间距    //note  暂时还不理解右间距与字符间距有什么区别，先与字符间距同样处理
			esc_sts[current_channel].charspace = chs[1];
			break;
		case '!':
			chs[1] = Getchar();
			//ESC ! n  设置打印机模式
			//根据n 的值设置字符打印模式
			//Bit	Off/On	Hex	功   能
			//0	-	-	暂无定义
			//1	off	0x00	解除反白模式
			//	on	0x02	设置反白模式
			//2	off	0x00	解除斜体模式
			//	on	0x04	设置斜体模式
			//3	-	-	暂无定义
			//4	off	0x00	解除倍高模式
			//	on	0x10	设置倍高模式
			//5	off	0x00	解除倍宽模式
			//	on	0x20	设置倍宽模式
			//6	-	-	暂无定义
			//7	off	0x00	解除下划线模式
			//	on	0x80	设置下划线模式
			esc_sts[current_channel].revert = ((chs[1]&(1<<1))?1:0);
			esc_sts[current_channel].italic = ((chs[1]&(1<<2))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<4))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<5))?1:0)<<4;
			esc_sts[current_channel].underline |= (((chs[1]&(1<<7))?1:0)|0x80);
			break;
		case '$':
			//ESC $ nl nh  设置绝对打印位置
			chs[1] = Getchar();
			chs[2] = Getchar();
			if (((chs[2]<<8)|chs[1]) < LineDot)
			{
				esc_sts[current_channel].start_dot = ((chs[2]<<8)|chs[1]);
			}
			break;
		case 0x2D:
			//ESC - n	选择/取消下划线模式  低半字节的低2位表示下划线宽度  0： 不改变  1：一点行   2:2点行
			//								 高半字节的低2位表示是否需要下划线  0： 取消  else: 选择
			chs[1] = Getchar();
			if ((chs[1] == 0)|| (chs[1] == 1) || (chs[1] == 2) || (chs[1] == 48)|| (chs[1] == 49) || (chs[1] == 50))
			{
				if(chs[1]&0x03)
				{
					esc_sts[current_channel].underline &= ~0x03;
					esc_sts[current_channel].underline |= (chs[1]&0x03);
				}

				if (chs[1]&0x30)
				{
					esc_sts[current_channel].underline |= 0x80;
				}
				else
				{
					esc_sts[current_channel].underline &= ~0x80;
				}
			}
			break;
		case '2':
			//ESC 2   设置默认行间距
			esc_sts[current_channel].linespace = 30;	//此数据是否是默认的3.75mm间距还有待测试！！！
			break;
		case '3':
			//ESC 3 n  设置默认行间距
			chs[1] = Getchar();
			esc_sts[current_channel].linespace = chs[1];
			break;
		case '@':
			//ESC @  初始化打印机
			esc_p_init(current_channel);
			//PrintBufToZero();		//这条命令挺奇葩的，小度掌柜经常卡住就是这个原因，不需要清除打印缓冲区
		case 'D':
			//ESC D n1....nk NULL 设置横向跳格位置
			memset(esc_sts[current_channel].tab,0,8);
			for (i = 0;i < 8;i++)
			{
				chs[1+i] = Getchar();
				if (chs[1+i] == 0)
				{
					break;
				}
				else
				{
					if (i == 0)
					{
						esc_sts[current_channel].tab[i] = chs[1+i];
					}
					else
					{
						if (chs[1+i] > chs[i])
						{
							esc_sts[current_channel].tab[i] = chs[1+i];
						}
					}	
				}
			}

			if (i == 8)
			{
				chs[1+i] = Getchar();	//0
			}

			break;
		case 'J':
			//ESC J n 打印并走纸
			chs[1] = Getchar();
			PrintCurrentBuffer(0);
			TPFeedLine(chs[1]);
			esc_sts[current_channel].start_dot = 0;
			break;
		case 'a':
			//ESC a n  选择对齐方式
			/*n的取值与对齐方式对应关系如下：
			n		对齐方式
			0，48	左对齐
			1，49	中间对齐
			2，50	右对齐*/
			chs[1] = Getchar();
			if ((chs[1] == 0)|| (chs[1] == 1) || (chs[1] == 2) || (chs[1] == 48) || (chs[1] == 49) || (chs[1] == 50))
			{
				esc_sts[current_channel].align = chs[1]&0x03;
			}
			break;
		case 'c':
			//ESC c 5 n 允许/禁止按键
			chs[1] = Getchar();
			if (chs[1] == '5')
			{
				chs[2] = Getchar();
				//@todo.... 设置按键禁止标志位

			}
			break;
		case 'd':
			//ESC d n 打印并向前走纸n 行
			chs[1] = Getchar();
			esc_sts[current_channel].start_dot = 0;
			PrintCurrentBuffer(0);
			TPFeedLine(chs[1]);
			break;
		case 'p':
			//ESC p m t1 t2 产生钱箱控制脉冲
			/*输出由t1 和t2 设定的钱箱开启脉冲到由m 指定的引脚：
			m	
			0,48		钱箱插座的引脚2
			1,49		钱箱插座的引脚5
			钱箱开启脉冲高电平时间为[t1*2ms],低电平时间为[t2*2ms]
			如果t2<t1,低电平时间为[t1*2ms]*/
			chs[1] = Getchar();
			if ((chs[1] == 0)||(chs[1] == 1)||(chs[1] == 48)||(chs[1] == 49))
			{
				chs[2] = Getchar();
				chs[3] = Getchar();
				//@todo...., 产生控制钱箱的脉冲

			}
			break;
		case 0x27:
			//ESC ’ml mh l1 h1 l2 h2 l3 h3 … li hi…  打印曲线
			//ml+mh*256表示这一行需要打印的点数，后面跟的（li+hi*256）是此行内需要打印的点的位置
			//实际上通过此指令可以打印位图
			chs[1] = Getchar();
			chs[2] = Getchar();
			cnt = chs[2]<<8;
			cnt |= chs[1];
			memset(tmp,0,sizeof(tmp));
			if (cnt<=LineDot)
			{
				for (i = 0; i < cnt;i++)
				{
					chs[1] = Getchar();
					chs[2] = Getchar();
					off = chs[2]<<8;
					off |= chs[1];
					tmp[off/8] |= (1<<(off%8)); 
				}
				TPPrintLine(tmp);
			}
			break;
		case '{':
			//ESC { n 打开/关闭颠倒打印模式
			chs[1] = Getchar();
			if (chs[1]&0x01)
			{
				esc_sts[current_channel].rotate = CIR_TWO_NINETY_DEGREE;
			}
			else
			{
				esc_sts[current_channel].rotate = ANTITYPE;
			}
			
		default:
			break;
		}
		break;
	case DC2:
		//uint8_t chs[5];
		chs[0] = cmd;
		chs[1] = Getchar();
		if (chs[1] == '~')
		{
			chs[2] = Getchar();
			//DC2 ~ n设定打印浓度...
			//@todo.....
		}
		else if (chs[1] == 'm')
		{
			chs[2] = Getchar();
			chs[3] = Getchar();
			chs[4] = Getchar();
			//DC2 m s x y黑标位置检测
			//@todo......
		}
		break;
	case DC3:
		//uint8_t chs[25];
		chs[0] = Getchar();
		switch(chs[0])
		{
		case 'A':
			chs[1] = Getchar();
			if (chs[1] == 'Q')
			{
				//DC3 A Q n d1 ... dn			设置蓝牙名称
				chs[2] = Getchar();
				if (chs[2] < 16)
				{
					for (i = 0; i < chs[2]; i++)
					{
						chs[6+i] = Getchar();
					}
					chs[6+i] = 0;

					if (BT816_set_name(current_channel,&chs[6]) == 0)
					{
						memcpy(chs,"+NAME=",6);
						chs[6+i] = ',';
						chs[7+i] = 'O';
						chs[8+i] = 'K';
						chs[9+i] = 0;

						PrintCurrentBuffer(0);	//先将缓冲区的内容打印出来，再打印设置字符串

						memset(tmp,0,LineDot/8);
						strcpy(tmp,chs);
						TPPrintLine(tmp);
						TPFeedLine(4);
					}
				}
			}
			else if (chs[1] == 'W')
			{
				//DC3 A W n d1 ... dn			设置蓝牙PIN
				chs[2] = Getchar();
				if (chs[2] < 8)
				{
					for (i = 0; i < chs[2]; i++)
					{
						chs[5+i] = Getchar();
					}
					chs[5+i] = 0;

					if (BT816_set_pin(current_channel,&chs[5]) == 0)
					{
						memcpy(chs,"+PIN=",5);
						chs[5+i] = ',';
						chs[6+i] = 'O';
						chs[7+i] = 'K';
						chs[8+i] = 0;

						PrintCurrentBuffer(0);	//先将缓冲区的内容打印出来，再打印设置字符串

						memset(tmp,0,LineDot/8);
						strcpy(tmp,chs);
						TPPrintLine(tmp);
						TPFeedLine(4);
					}
				}
			}
			else if (chs[1] == 'a')
			{
				chs[2] = Getchar();
				//DC3 A a n设置蓝牙传输速率
				//@todo....

			}
			break;
		case 'B':
			chs[1] = Getchar();
			if (chs[1] == 'R')
			{
				chs[2] = Getchar();
				//DC3 B R n  设定串口波特率
				//@todo......
			}
			break;
		case 'r':
			//DC3  r  返回8个字节的产品ID
			BT816_send_data(current_channel,"HJ_BTPr1",8);
			break;
		case 's':
			//DC3 s  查询打印机状态
			//返回1个字节
			//	n=0打印机有值    n=4打印机缺纸  n=8准备打印      
			//	n='L' 电压过低(5.0Volt 以下)       n='O' 电压过高(9.5Volt 以上)
			//@todo...
			//BT816_send_data(current_channel,&esc_sts[current_channel].status4,1);
			break;
		case 'L':
			chs[1] = Getchar();
			chs[2] = Getchar();
			//DC3 L x y  设置字符间距和行间距，默认0
			if (chs[1]<128)
			{
				esc_sts[current_channel].charspace = chs[1];
			}
			if (chs[2]<128)
			{
				esc_sts[current_channel].linespace = chs[2];
			}
			break;
		}
		break;
	case FS:		// FS
		chs[0] = Getchar();
		if (chs[0] == 'p')
		{
			//FS  p  n  m 打印下载到FLASH 中的位图
			//@todo....
		}
		else if (chs[0] == 'q')
		{
			//FS q n [xL xH yL yH d1...dk]1...[xL xH yL yH d1...dk]n 定义Flash 位图
			//@todo....
		}
		break;
	case GS:		// GS
		chs[0] = Getchar();
		switch(chs[0])
		{
		case 0x0c:
			//GS FF  走纸到黑标
			//@todo....

			break;
		case '!':
			//GS ! n 选择字符大小
			chs[1]=Getchar();
			esc_sts[current_channel].larger |= ((chs[1]&(1<<4))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<5))?1:0)<<4;
			if (chs[1]&0x0f)
			{
				esc_sts[current_channel].larger |= 0x01;		//暂时只支持2倍高
			}
			else
			{
				esc_sts[current_channel].larger &= ~0x01;		
			}
			if (chs[1]&0xf0)
			{
				esc_sts[current_channel].larger |= (0x01<<4);	//暂时支持2倍宽
			}
			else
			{
				esc_sts[current_channel].larger &= ~(0x01<<4);	
			}
			break;
		case '*':
			//GS * x y d1...d(x × y × 8) 定义下载位图,此位图下载到RAM区域
			//@todo....
			break;
		case 0x2f:
			//GS  /  m 打印下载到RAM中的位图
			//@todo....
			break;
		case 'B':
			//GS  B  n 选择/ 取消黑白反显打印模式
			chs[1] = Getchar();
			esc_sts[current_channel].revert = (chs[1]&0x01);
			break;
		case 'H':
			//GS  H  n 选择HRI 字符的打印位置
			chs[1] = Getchar();
			//@todo.....  设置HRI字符的打印位置
			break;
		case 'L':
			//GS  L  nL  nH 设置左边距
			chs[1] = Getchar();
			chs[2] = Getchar();
			esc_sts[current_channel].leftspace = chs[2];
			esc_sts[current_channel].leftspace <<= 8;
			esc_sts[current_channel].leftspace |= chs[1];
			break;
		case 'h':
			//GS  h  n 选择条码高度
			chs[1] = Getchar();
			//@todo...   设置条码打印的高度
			break;
		case 'k':
			//①GS k m d1...dk NUL②GS k m n d1...dn 打印条码
			//@todo....   打印条码
			chs[1] = Getchar();
			if (chs[1] <= 6)
			{
				//第①种命令
				if ((chs[1] == 0)||(chs[1] == 1))
				{
					for (i = 0; i < 12;i++)
					{
						//todo.... 接收到的数据待处理！！！
						Getchar();
					}
				}
				else if (chs[1] == 2)
				{
					for (i = 0; i < 13;i++)
					{
						//todo.... 接收到的数据待处理！！！
						Getchar();
					}
				}
				else if (chs[1] == 3)
				{
					for (i = 0; i < 8;i++)
					{
						//todo.... 接收到的数据待处理！！！
						Getchar();
					}
				}
				else
				{
					//todo....
					do 
					{
						if (Getchar() == 0)
						{
							break;
						}
					} while (1);
				}
			}
			else if ((chs[1]>=65)&&(chs[1]<=73))
			{
				//第②种命令
				n = Getchar();
				if ((chs[1] == 65)||(chs[1] == 66))
				{
					n = 12;
				}
				else if (chs[1] == 67)
				{
					n = 13;
				}
				else if (chs[1] == 68)
				{
					n = 8;
				}

				for (i = 0; i < n; i++)
				{
					//@todo....
					Getchar();
				}
			}
			break;
		case 'v':
			//GS v 0 m xL xH yL yH d1...dk 打印光栅位图
			//@todo....
			chs[1] = Getchar();
			if (chs[1] == '0')
			{
				chs[2] = Getchar();
				if ((chs[2]<=3)||((chs[2]>=48)&&(chs[2]<=51)))
				{
					chs[3] = Getchar();
					chs[4] = Getchar();
					chs[5] = Getchar();
					chs[6] = Getchar();

					for (i = 0; i<((chs[4]*256+chs[3])*(chs[6]*256+chs[5]));i++)
					{
						//@todo....保存光栅位图并打印
						Getchar();
					}
				}
			}
			break;
		case 'w':
			//GS w n 设置条码宽度
			chs[1] = Getchar();
			//@todo....
			break;
		case 'N':
			//GS N m e nH nL d1...dn 打印QR二维码
			//@todo....
			chs[1] = Getchar();
			chs[2] = Getchar();
			chs[3] = Getchar();
			chs[4] = Getchar();
			for (i = 0;i<(chs[4]*256+chs[3]);i++)
			{
				//@todo....
				Getchar();
			}
			break;
		case 'j':
			//GS j m 即时打印条码位置
			chs[1] = Getchar();
			//@todo....
			break;
		}
		break;
#endif
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

