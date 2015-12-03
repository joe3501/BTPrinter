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
signed char	 current_channel;		//��ǰ���ڴ����ͨ��

extern void esc_p_init(unsigned int n)
{
	uint8_t i;
	//----chang

	esc_sts[n].international_character_set = 0;    // english
	esc_sts[n].character_code_page = g_param.character_code_page;

	esc_sts[n].prt_on = 0;
	esc_sts[n].larger = 0;
#ifdef ASCII9X24
	esc_sts[n].font_en = FONT_B_WIDTH;	// ����
#else
	esc_sts[n].font_en = FONT_A_WIDTH;	// ����
#endif
	esc_sts[n].font_cn = FONT_CN_A_WIDTH;	// ����
	esc_sts[n].bold = 0;		// ����
	esc_sts[n].italic = 0;		// б��
	esc_sts[n].double_strike=0;//�ص���ӡ
	esc_sts[n].underline = 0;	// �»���
	esc_sts[n].revert = 0;		// ������ʾ
	esc_sts[n].rotate = 0;
	esc_sts[n].start_dot = 0;
	esc_sts[n].smoothing_mode = 0;	// ƽ��ģʽ
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
	esc_sts[n].upside_down=0;//����
	esc_sts[n].barcode_height = 50;
	esc_sts[n].barcode_width = 2;
	esc_sts[n].barcode_leftspace = 0;
	esc_sts[n].barcode_char_pos = 0;//����ʾ
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
			//ESC SP n �����ַ��Ҽ��    //note  ��ʱ��������Ҽ�����ַ������ʲô���������ַ����ͬ������
			esc_sts[current_channel].charspace = chs[1];
			break;
		case '!':
			chs[1] = Getchar();
			//ESC ! n  ���ô�ӡ��ģʽ
			//����n ��ֵ�����ַ���ӡģʽ
			//Bit	Off/On	Hex	��   ��
			//0	-	-	���޶���
			//1	off	0x00	�������ģʽ
			//	on	0x02	���÷���ģʽ
			//2	off	0x00	���б��ģʽ
			//	on	0x04	����б��ģʽ
			//3	-	-	���޶���
			//4	off	0x00	�������ģʽ
			//	on	0x10	���ñ���ģʽ
			//5	off	0x00	�������ģʽ
			//	on	0x20	���ñ���ģʽ
			//6	-	-	���޶���
			//7	off	0x00	����»���ģʽ
			//	on	0x80	�����»���ģʽ
			esc_sts[current_channel].revert = ((chs[1]&(1<<1))?1:0);
			esc_sts[current_channel].italic = ((chs[1]&(1<<2))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<4))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<5))?1:0)<<4;
			esc_sts[current_channel].underline |= (((chs[1]&(1<<7))?1:0)|0x80);
			break;
		case '$':
			//ESC $ nl nh  ���þ��Դ�ӡλ��
			chs[1] = Getchar();
			chs[2] = Getchar();
			if (((chs[2]<<8)|chs[1]) < LineDot)
			{
				esc_sts[current_channel].start_dot = ((chs[2]<<8)|chs[1]);
			}
			break;
		case 0x2D:
			//ESC - n	ѡ��/ȡ���»���ģʽ  �Ͱ��ֽڵĵ�2λ��ʾ�»��߿��  0�� ���ı�  1��һ����   2:2����
			//								 �߰��ֽڵĵ�2λ��ʾ�Ƿ���Ҫ�»���  0�� ȡ��  else: ѡ��
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
			//ESC 2   ����Ĭ���м��
			esc_sts[current_channel].linespace = 30;	//�������Ƿ���Ĭ�ϵ�3.75mm��໹�д����ԣ�����
			break;
		case '3':
			//ESC 3 n  ����Ĭ���м��
			chs[1] = Getchar();
			esc_sts[current_channel].linespace = chs[1];
			break;
		case '@':
			//ESC @  ��ʼ����ӡ��
			esc_p_init(current_channel);
			//PrintBufToZero();		//��������ͦ����ģ�С���ƹ񾭳���ס�������ԭ�򣬲���Ҫ�����ӡ������
		case 'D':
			//ESC D n1....nk NULL ���ú�������λ��
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
			//ESC J n ��ӡ����ֽ
			chs[1] = Getchar();
			PrintCurrentBuffer(0);
			TPFeedLine(chs[1]);
			esc_sts[current_channel].start_dot = 0;
			break;
		case 'a':
			//ESC a n  ѡ����뷽ʽ
			/*n��ȡֵ����뷽ʽ��Ӧ��ϵ���£�
			n		���뷽ʽ
			0��48	�����
			1��49	�м����
			2��50	�Ҷ���*/
			chs[1] = Getchar();
			if ((chs[1] == 0)|| (chs[1] == 1) || (chs[1] == 2) || (chs[1] == 48) || (chs[1] == 49) || (chs[1] == 50))
			{
				esc_sts[current_channel].align = chs[1]&0x03;
			}
			break;
		case 'c':
			//ESC c 5 n ����/��ֹ����
			chs[1] = Getchar();
			if (chs[1] == '5')
			{
				chs[2] = Getchar();
				//@todo.... ���ð�����ֹ��־λ

			}
			break;
		case 'd':
			//ESC d n ��ӡ����ǰ��ֽn ��
			chs[1] = Getchar();
			esc_sts[current_channel].start_dot = 0;
			PrintCurrentBuffer(0);
			TPFeedLine(chs[1]);
			break;
		case 'p':
			//ESC p m t1 t2 ����Ǯ���������
			/*�����t1 ��t2 �趨��Ǯ�俪�����嵽��m ָ�������ţ�
			m	
			0,48		Ǯ�����������2
			1,49		Ǯ�����������5
			Ǯ�俪������ߵ�ƽʱ��Ϊ[t1*2ms],�͵�ƽʱ��Ϊ[t2*2ms]
			���t2<t1,�͵�ƽʱ��Ϊ[t1*2ms]*/
			chs[1] = Getchar();
			if ((chs[1] == 0)||(chs[1] == 1)||(chs[1] == 48)||(chs[1] == 49))
			{
				chs[2] = Getchar();
				chs[3] = Getchar();
				//@todo...., ��������Ǯ�������

			}
			break;
		case 0x27:
			//ESC ��ml mh l1 h1 l2 h2 l3 h3 �� li hi��  ��ӡ����
			//ml+mh*256��ʾ��һ����Ҫ��ӡ�ĵ�����������ģ�li+hi*256���Ǵ�������Ҫ��ӡ�ĵ��λ��
			//ʵ����ͨ����ָ����Դ�ӡλͼ
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
			//ESC { n ��/�رյߵ���ӡģʽ
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
			//DC2 ~ n�趨��ӡŨ��...
			//@todo.....
		}
		else if (chs[1] == 'm')
		{
			chs[2] = Getchar();
			chs[3] = Getchar();
			chs[4] = Getchar();
			//DC2 m s x y�ڱ�λ�ü��
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
				//DC3 A Q n d1 ... dn			������������
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

						PrintCurrentBuffer(0);	//�Ƚ������������ݴ�ӡ�������ٴ�ӡ�����ַ���

						memset(tmp,0,LineDot/8);
						strcpy(tmp,chs);
						TPPrintLine(tmp);
						TPFeedLine(4);
					}
				}
			}
			else if (chs[1] == 'W')
			{
				//DC3 A W n d1 ... dn			��������PIN
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

						PrintCurrentBuffer(0);	//�Ƚ������������ݴ�ӡ�������ٴ�ӡ�����ַ���

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
				//DC3 A a n����������������
				//@todo....

			}
			break;
		case 'B':
			chs[1] = Getchar();
			if (chs[1] == 'R')
			{
				chs[2] = Getchar();
				//DC3 B R n  �趨���ڲ�����
				//@todo......
			}
			break;
		case 'r':
			//DC3  r  ����8���ֽڵĲ�ƷID
			BT816_send_data(current_channel,"HJ_BTPr1",8);
			break;
		case 's':
			//DC3 s  ��ѯ��ӡ��״̬
			//����1���ֽ�
			//	n=0��ӡ����ֵ    n=4��ӡ��ȱֽ  n=8׼����ӡ      
			//	n='L' ��ѹ����(5.0Volt ����)       n='O' ��ѹ����(9.5Volt ����)
			//@todo...
			//BT816_send_data(current_channel,&esc_sts[current_channel].status4,1);
			break;
		case 'L':
			chs[1] = Getchar();
			chs[2] = Getchar();
			//DC3 L x y  �����ַ������м�࣬Ĭ��0
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
			//FS  p  n  m ��ӡ���ص�FLASH �е�λͼ
			//@todo....
		}
		else if (chs[0] == 'q')
		{
			//FS q n [xL xH yL yH d1...dk]1...[xL xH yL yH d1...dk]n ����Flash λͼ
			//@todo....
		}
		break;
	case GS:		// GS
		chs[0] = Getchar();
		switch(chs[0])
		{
		case 0x0c:
			//GS FF  ��ֽ���ڱ�
			//@todo....

			break;
		case '!':
			//GS ! n ѡ���ַ���С
			chs[1]=Getchar();
			esc_sts[current_channel].larger |= ((chs[1]&(1<<4))?1:0);
			esc_sts[current_channel].larger |= ((chs[1]&(1<<5))?1:0)<<4;
			if (chs[1]&0x0f)
			{
				esc_sts[current_channel].larger |= 0x01;		//��ʱֻ֧��2����
			}
			else
			{
				esc_sts[current_channel].larger &= ~0x01;		
			}
			if (chs[1]&0xf0)
			{
				esc_sts[current_channel].larger |= (0x01<<4);	//��ʱ֧��2����
			}
			else
			{
				esc_sts[current_channel].larger &= ~(0x01<<4);	
			}
			break;
		case '*':
			//GS * x y d1...d(x �� y �� 8) ��������λͼ,��λͼ���ص�RAM����
			//@todo....
			break;
		case 0x2f:
			//GS  /  m ��ӡ���ص�RAM�е�λͼ
			//@todo....
			break;
		case 'B':
			//GS  B  n ѡ��/ ȡ���ڰ׷��Դ�ӡģʽ
			chs[1] = Getchar();
			esc_sts[current_channel].revert = (chs[1]&0x01);
			break;
		case 'H':
			//GS  H  n ѡ��HRI �ַ��Ĵ�ӡλ��
			chs[1] = Getchar();
			//@todo.....  ����HRI�ַ��Ĵ�ӡλ��
			break;
		case 'L':
			//GS  L  nL  nH ������߾�
			chs[1] = Getchar();
			chs[2] = Getchar();
			esc_sts[current_channel].leftspace = chs[2];
			esc_sts[current_channel].leftspace <<= 8;
			esc_sts[current_channel].leftspace |= chs[1];
			break;
		case 'h':
			//GS  h  n ѡ������߶�
			chs[1] = Getchar();
			//@todo...   ���������ӡ�ĸ߶�
			break;
		case 'k':
			//��GS k m d1...dk NUL��GS k m n d1...dn ��ӡ����
			//@todo....   ��ӡ����
			chs[1] = Getchar();
			if (chs[1] <= 6)
			{
				//�ڢ�������
				if ((chs[1] == 0)||(chs[1] == 1))
				{
					for (i = 0; i < 12;i++)
					{
						//todo.... ���յ������ݴ���������
						Getchar();
					}
				}
				else if (chs[1] == 2)
				{
					for (i = 0; i < 13;i++)
					{
						//todo.... ���յ������ݴ���������
						Getchar();
					}
				}
				else if (chs[1] == 3)
				{
					for (i = 0; i < 8;i++)
					{
						//todo.... ���յ������ݴ���������
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
				//�ڢ�������
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
			//GS v 0 m xL xH yL yH d1...dk ��ӡ��դλͼ
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
						//@todo....�����դλͼ����ӡ
						Getchar();
					}
				}
			}
			break;
		case 'w':
			//GS w n ����������
			chs[1] = Getchar();
			//@todo....
			break;
		case 'N':
			//GS N m e nH nL d1...dn ��ӡQR��ά��
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
			//GS j m ��ʱ��ӡ����λ��
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
					// GB18030�����4�ֽ���չ
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
				// GB13000�����2�ֽ���չ
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

