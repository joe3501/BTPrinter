#include "Type.h"
#include "Esc_p.h"
#include "font.h"
//#include "font8x16.h"
#include "res_spi.h"
#include "config.h"
#include "DotFill.h"
#include "basic_fun.h"
#include <string.h>
// #define FONT_BYTE_SWAP
//======================================================================================================
//#define MAX_GBK_ADDR                0x183870
//#define GB2312_16x16_START_ADDR     0x183870

//////////////////////////////////////////////////////////////////////////////

static void ByteSwap(uint8_t  *p, uint8_t len)
{
	uint8_t  ByteSwapTable[] =
	{
		0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
		0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
		0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
		0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
		0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
		0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
		0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
		0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
		0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
		0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
		0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
		0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
		0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
		0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
		0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
		0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF,
	};

	do
	{
		*p = ByteSwapTable[*p];
		p++;
	}while(--len);
}
//======================================================================================================
//	字体加粗
//======================================================================================================
static void FontBold(uint8_t font)
{
    uint32_t i,j,k;
	if ((esc_sts[current_channel].bold) || (esc_sts[current_channel].double_strike))
	{
        switch(font)
            {
            case FONT_A_WIDTH:  // FONT A
                for(i=FONT_A_WIDTH-1;i;i--)
                {
                    for (j=0;j<FONT_A_HEIGHT/8;j++)
                    {
                        k= 7;
                        do{
                            if(esc_sts[current_channel].font_buf.font_a[i-1][j]&(1<<k))
                            {
                               esc_sts[current_channel].font_buf.font_a[i][j] |=(1<<k);
                            }
                        }while(k--);
                    }
                }
                break;
            case FONT_B_WIDTH:  // FONT B
                for(i=FONT_B_WIDTH-1;i;i--)
                {
                    for (j=0;j<FONT_B_HEIGHT/8;j++)
                    {
                        k= 7;
                        do{
                            if(esc_sts[current_channel].font_buf.font_b[i-1][j]&(1<<k))
                            {
                               esc_sts[current_channel].font_buf.font_b[i][j] |=(1<<k);
                            }
                        }while(k--);
                    }
                }
                break;
            case FONT_CN_A_WIDTH:  // FONT A
                for(i=FONT_CN_A_WIDTH-1;i;i--)
                {
                    for (j=0;j<FONT_CN_A_HEIGHT/8;j++)
                    {
                        k= 7;
                        do{
                            if(esc_sts[current_channel].font_buf.font_cn_a[i-1][j]&(1<<k))
                            {
                               esc_sts[current_channel].font_buf.font_cn_a[i][j] |=(1<<k);
                            }
                        }while(k--);
                    }
                }
                break;
            case FONT_CN_B_WIDTH:  // FONT B
                for(i=FONT_CN_B_WIDTH-1;i;i--)
                {
                    for (j=0;j<FONT_CN_B_HEIGHT/8;j++)
                    {
                        k= 7;
                        do{
                            if(esc_sts[current_channel].font_buf.font_cn_b[i-1][j]&(1<<k))
                            {
                               esc_sts[current_channel].font_buf.font_cn_b[i][j] |=(1<<k);
                            }
                        }while(k--);
                    }
                }
                break;
            }
	}
}

//======================================================================================================
//	字体放大
//======================================================================================================
extern void FontEnlarge(uint8_t *dot, uint8_t *buf, uint8_t row)          //纵向放大，变高
{
	uint8_t r, ratio;
	uint8_t i, j;
	uint16_t bit;
	uint8_t c;

	ratio = (esc_sts[current_channel].larger & 0x0f)+1;                          //高放大比例
	if (ratio == 1)
	{
		memcpy(dot, buf, row);
		return;
	}
	bit = 0;
	//memset(dot, 0, row * (ratio+1));
	memset(dot, 0, row * ratio);
	for (i=0; i<row; i++)              //纵向有三个字节，每次一个一个字节取
	{
		c = *buf++;                   //51中数据是一个一个字节存的
		j = 7;
		do
		{
			if (c & (1 << j))
			{
				r = 0;
				do
				{
					dot[bit >> 3] |= (1 << (7-(bit & 0x07)));
					bit++;
				}
				while ((++r) < ratio);
			}
			else
			{
				bit += ratio;
			}
		}
		while (j--);
	}
//	printf("fang da");
}
//======================================================================================================
//	字体平滑
//======================================================================================================
static void FontSmoothing(uint8_t font)
{
	if (esc_sts[current_channel].smoothing_mode)
	{

	}
}

//======================================================================================================
//	下划线
//======================================================================================================
//===> 从此函数可以推断出字模数据是从上往下扫描的，而且高位表示上面的点
extern void FontUnderline(uint16_t start_col, uint16_t end_col)
{
	uint8_t c;

	if ((esc_sts[current_channel].rotate) || (esc_sts[current_channel].revert))
	{
		return;
	}
	switch (esc_sts[current_channel].underline)
	{
	case 0x81:                    //细下划线
		c = 0x01;
		break;
	case 0x82:                   //粗下划线
		c = 0x03;
		break;
	case 0:
	default:
		return;
	}

	for (; (start_col < end_col) && (start_col < ARRAY_SIZE(esc_sts[current_channel].dot)); start_col++)
	{
		esc_sts[current_channel].dot[start_col][ARRAY_SIZE(esc_sts[current_channel].dot[0])-1] |= c;
	}
}
//======================================================================================================
//	反白显示
//======================================================================================================
static void FontRevertProc(uint8_t *p, uint16_t len)
{
	uint8_t i;

	for (i=0; i<len; i++)
	{
		*p++ ^= 0xff;                                  //异或，如:1010 1100^1111 1111=0101 0011
	}
}

static void FontRevert(uint8_t font)
{
	if (esc_sts[current_channel].revert)
	{
		switch (font)
		{
		case FONT_A_WIDTH:
			FontRevertProc(&esc_sts[current_channel].font_buf.font_a[0][0], sizeof(esc_sts[current_channel].font_buf.font_a));
			break;
		case FONT_B_WIDTH:
			FontRevertProc(&esc_sts[current_channel].font_buf.font_b[0][0], sizeof(esc_sts[current_channel].font_buf.font_b));
			break;
#if defined(GB18030) || defined(GBK) || defined(GB2312)
		case FONT_CN_A_WIDTH:
			FontRevertProc(&esc_sts[current_channel].font_buf.font_cn_a[0][0], sizeof(esc_sts[current_channel].font_buf.font_cn_a));
			break;
		case FONT_CN_B_WIDTH:
			FontRevertProc(&esc_sts[current_channel].font_buf.font_cn_b[0][0], sizeof(esc_sts[current_channel].font_buf.font_cn_b));
			break;
#endif
		}
	}
}
//======================================================================================================
//	旋转
//======================================================================================================
extern void FontCircumvolve(uint8_t *dot, uint8_t *buf, uint8_t row,uint8_t col,uint16_t angle)//把buf的缓冲区转换后送入dot缓冲区，row高度，col宽度
{                               //在旋转九十和两百七十度时，我们的临时buffer要大一点
	//uint8_t temp;
	uint16_t bit;
	uint16_t i,j,k;
    uint16_t max_rowbyte;
    max_rowbyte= (row+7)/8;
	switch(angle)
	{
		case ANTITYPE:                                                     //原型
			for(i=0;i<col;i++)
	       		{
				for(j=0;j<max_rowbyte;j++)
				{
					*dot=*buf;
					dot++;
					buf++;
				}
	      	 	}
			 break;
		case CIR_NINETY_DEGREE:                                    //顺旋转九十度
			bit=0;
			for(i=max_rowbyte;i>0;i--)
			{
				k=0;
				do
				{
					for(j=0;j<col;j++)
					{
						if (buf[j*max_rowbyte+i-1] & (1<<k))
						{
							dot[bit>>3] |= (1<<(7-(bit&0x07)));
						}
                        			bit++;
						if(j == (col-1))
						{
							if(col & 0x07)
							{
								bit=bit+(8-(col&0x07));
							}
						}
					}
				}while((++k)<8);
			}
			break;
		case CIR_TWO_NINETY_DEGREE:                               //顺时钟旋转一百八十度
			for(i=col;i>0;i--)
			{
				for(j=max_rowbyte;j>0;j--)
				{
					k=7;
					do
					{
						if (buf[(i-1)*max_rowbyte+j-1] & (1<<k))
						{
							dot[(col-i)*max_rowbyte+(max_rowbyte-j)] |= (1<<(7-k));
						}
					}while(k--);

				}
			}
			break;
		case CIR_THREE_NINETY_DEGREE:                            //顺时钟旋转两百七十度
			bit=0;
			for(i=0;i<max_rowbyte;i++)
			{
				k=7;
				do
				{
					for(j=col;j>0;j--)
					{
						if (buf[(j-1)*max_rowbyte+i] & (1<<k))
						{
							dot[bit>>3] |= (1<<(7-(bit&0x07)));
						}
                        			bit++;
						if(j == 1)
						{
							bit=bit+(8-(col&0x07));
						}
					}
				}while(k--);
			}
			break;
		default :
			break;
	}

}

#if defined(GB18030) || defined(GBK) || defined(GB2312)
#if 0
static unsigned long gt (unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{
	unsigned long h=0;

	if (c2==0x7f) return (0);

	if ((c1 >= 0xa1) && (c1 <= 0xab) && (c2 >= 0xa1)) //Section 1
	{
		h = (c1 - 0xa1) * 94 + (c2 - 0xa1);
	}
	else if ((c1 >= 0xa8) && (c1 <= 0xa9) && (c2 < 0xa1)) //Section 5
	{
		if (c2>0x7f)
		{
			c2--;
		}
		h = (c1-0xa8)*96 + (c2-0x40)+846;
	}
	if ((c1 >= 0xb0) && (c1 <= 0xf7) && (c2 >= 0xa1)) //Section 2
	{
		h = (c1 - 0xb0) * 94 + (c2 - 0xa1)+1038;
	}
	else if ((c1 < 0xa1) && (c1 >= 0x81)) //Section 3
	{
		if (c2 > 0x7f)
		{
			c2--;
		}
		h = (c1-0x81)*190 + (c2-0x40) + 1038 +6768;
	}
	else if ((c1>=0xaa) && (c2<0xa1)) //Section 4
	{
		if (c2 > 0x7f)
		{
			c2--;
		}
		h = (c1-0xaa)*96 + (c2-0x40) + 1038 +12848;
	}
	if ((c2 >= 0x30) && (c2 <= 0x39)) //Extended Section (With 4 BYTES InCode)
	{
		if ((c4 < 0x30) || (c4 > 0x39))
		{
			return (0);
		}
		h=(c3-0x81)*12600+(c4-0x39)*1260+(c1-0xee)*10+(c2-0x39)+22046;
		if (h>=22046 && h<=22046+6530)
		{
			return(h);
		}
		else
		{
			return (0);
		}
	}
	return(h);
}
#endif
#endif
//======================================================================================================
//----chang
#define CODE_PAGE_START_ADR     (0x3600UL)
#define FONTEN_9X24_ADR         (0x50000UL)
#define CODE_PAGE_START_ADR_9_24     (0X50A20UL)

#if defined(CODEPAGE)

static uint8_t ChToInternaionalCharSetIdx(uint8_t ch)
{
    switch(ch)
    {
        case 0x23:
            return 0;
        case 0x24:
            return 1;
        case 0x40:
            return 2;
        case 0x5B:
            return 3;
        case 0x5C:
            return 4;
        case 0x5D:
            return 5;
        case 0x5E:
            return 6;
        case 0x60:
            return 7;
        case 0x7B:
            return 8;
        case 0x7C:
            return 9;
        case 0x7D:
            return 10;
        case 0x7E:
            return 11;
        default:
            return 0xff;
    }
}
#endif

extern void GetEnglishFont(uint8_t ascii)
{

	uint8_t buf1[FONT_A_HEIGHT*((FONT_A_WIDTH+7)/8)];
	switch (esc_sts[current_channel].font_en)
	{
	case FONT_A_WIDTH:	// FONT A
    //----chang
          {
            #if defined(CODEPAGE)
            if(ChToInternaionalCharSetIdx(ascii)!= 0xff)
            {
                F25L_Read(((uint16_t)ChToInternaionalCharSetIdx(ascii))*FONT_A_WIDTH*((FONT_A_HEIGHT+7)/8) + esc_sts[current_channel].international_character_set*12UL*FONT_A_WIDTH*((FONT_A_HEIGHT+7)/8),
                                    esc_sts[current_channel].font_buf.font_a[0], sizeof(esc_sts[current_channel].font_buf.font_a));
            }
            else
            #endif
            {
                #if defined(CODEPAGE)
                if(ascii < 0x80)
                {
                    F25L_Read((ascii)*FONT_A_WIDTH*((FONT_A_HEIGHT+7)/8) + font_en_addr, esc_sts[current_channel].font_buf.font_a[0], sizeof(esc_sts[current_channel].font_buf.font_a));       // get font from sflash
                }
                else
                {
                    uint32_t adr;
                    adr = CODE_PAGE_START_ADR + esc_sts[current_channel].character_code_page *(0x80UL*sizeof(esc_sts[current_channel].font_buf.font_a))+ (ascii - 0x80)*sizeof(esc_sts[current_channel].font_buf.font_a);
                    F25L_Read(adr, esc_sts[current_channel].font_buf.font_a[0], sizeof(esc_sts[current_channel].font_buf.font_a));       // get font from sflash

                }
    			// TODO: esc_sts[current_channel].international_character_set
                #elif defined(FONT9x24)
                if(ascii >= 0x20 && ascii<0x80)
                {
                    memcpy(esc_sts[current_channel].font_buf.font_a, (const void*)(FontList[FNT_ASC_9_24].address)[ascii-0x20], sizeof(esc_sts[current_channel].font_buf.font_a));
                }
                else
                {
                    memset(esc_sts[current_channel].font_buf.font_a, 0xff, sizeof(esc_sts[current_channel].font_buf.font_a));
                }
                #else
    			memcpy(esc_sts[current_channel].font_buf.font_a, (const unsigned char*)(FontList[FNT_ASC_12_24].address)+(ascii-0x20)*FontList[FNT_ASC_12_24].size, sizeof(esc_sts[current_channel].font_buf.font_a));
                #endif
	         	}
          }
		 break;
	case FONT_B_WIDTH:	// FONT B
		{
			// TODO: esc_sts[current_channel].international_character_set
			memcpy(esc_sts[current_channel].font_buf.font_b, (const unsigned char*)(FontList[FNT_ASC_8_16].address)+(ascii-0x20)*FontList[FNT_ASC_8_16].size, sizeof(esc_sts[current_channel].font_buf.font_b));
            #ifdef ASCII9X24
            LeftRightSwap(esc_sts[current_channel].font_buf.font_b[0], sizeof(esc_sts[current_channel].font_buf.font_b));
            #endif
		}
		break;
	}
	FontBold(esc_sts[current_channel].font_en);
	FontSmoothing(esc_sts[current_channel].font_cn);
	FontRevert(esc_sts[current_channel].font_en);


	// TODO: 需要考虑放大后的位图大小


	switch (esc_sts[current_channel].font_en)
	{
	case FONT_A_WIDTH:	// FONT A
		memset(buf1,0,sizeof(buf1));
		if(esc_sts[current_channel].rotate==1||esc_sts[current_channel].rotate==3)
		{
			FontCircumvolve(&buf1[0], &esc_sts[current_channel].font_buf.font_a[0][0], FONT_A_HEIGHT,FONT_A_WIDTH,esc_sts[current_channel].rotate);
			DotFillToBuf(&buf1[0], FONT_A_HEIGHT,( FONT_A_WIDTH+7)/8*8, 1);
		}
		else
		{
			FontCircumvolve(&buf1[0], &esc_sts[current_channel].font_buf.font_a[0][0],FONT_A_HEIGHT,FONT_A_WIDTH,esc_sts[current_channel].rotate);
			DotFillToBuf(&buf1[0], FONT_A_WIDTH,( FONT_A_HEIGHT+7)/8*8, 1);
		}
		break;
	case FONT_B_WIDTH:	// FONT B
		DotFillToBuf(&esc_sts[current_channel].font_buf.font_b[0][0], FONT_B_WIDTH, FONT_B_HEIGHT, 1);
		break;

    #if defined(CODEPAGE)
    case FONT_C_WIDTH:
        DotFillToBuf(&esc_sts[current_channel].font_buf.font_c[0][0], FONT_C_WIDTH, FONT_C_HEIGHT, 1);
     #endif



	}


}
//======================================================================================================
#if defined(GB18030) || defined(GBK) || defined(GB2312)
extern void GetChineseFont(uint8_t *c, uint8_t charset)
{
	//uint32_t loc;
	uint8_t buf2[FONT_CN_A_HEIGHT*((FONT_CN_A_WIDTH+7)/8)];
	switch (esc_sts[current_channel].font_cn)
	{
	case FONT_CN_A_WIDTH:
		{
			//switch (charset)
			//{
			//case CHINESE_FONT_GB2312:
			//case CHINESE_FONT_GB13000:
			//default:
			//	loc = gt(c[0], c[1], 0, 0) * sizeof(esc_sts[current_channel].font_buf.font_cn_a);
			//	break;
			//case CHINESE_FONT_GB18030:
			//	loc = gt(c[0], c[1], c[2], c[3]) * sizeof(esc_sts[current_channel].font_buf.font_cn_a);
			//	break;
			//}
			font_data_read(FNT_CHN_24_24, c,esc_sts[current_channel].font_buf.font_cn_a[0], sizeof(esc_sts[current_channel].font_buf.font_cn_a));		// get font from sflash
			//ByteSwap(esc_sts[current_channel].font_buf.font_cn_a[0], sizeof(esc_sts[current_channel].font_buf.font_cn_a));
		}
		break;
	case FONT_CN_B_WIDTH:
		{
			// TODO: change flash offset
			//loc = GB2312_16x16_START_ADDR + ((c[0]-0xA1)*(0xfe - 0xA0) + (c[1]-0xA1) )* sizeof(esc_sts[current_channel].font_buf.font_cn_b);//GB2312
			font_data_read(FNT_CHN_16_16, c,esc_sts[current_channel].font_buf.font_cn_b[0], sizeof(esc_sts[current_channel].font_buf.font_cn_b));
		}
		break;
	}
	FontBold(esc_sts[current_channel].font_cn);
	FontSmoothing(esc_sts[current_channel].font_cn);
	FontRevert(esc_sts[current_channel].font_cn);
	// TODO: 需要考虑放大后的位图大小
	switch (esc_sts[current_channel].font_cn)
	{
	case FONT_CN_A_WIDTH:	// FONT A
		memset(buf2,0,sizeof(buf2));
		if(esc_sts[current_channel].rotate==1||esc_sts[current_channel].rotate==3)
		{
			FontCircumvolve(&buf2[0], &esc_sts[current_channel].font_buf.font_cn_a[0][0], FONT_CN_A_HEIGHT,FONT_CN_A_WIDTH,esc_sts[current_channel].rotate);
			DotFillToBuf(&buf2[0], FONT_CN_A_HEIGHT,( FONT_CN_A_WIDTH+7)/8*8, 1);
		}
		else
		{
			FontCircumvolve(&buf2[0], &esc_sts[current_channel].font_buf.font_cn_a[0][0],FONT_CN_A_HEIGHT,FONT_CN_A_WIDTH,esc_sts[current_channel].rotate);
			DotFillToBuf(&buf2[0], FONT_CN_A_WIDTH, ( FONT_CN_A_HEIGHT+7)/8*8, 1);
		}
		break;
	case FONT_CN_B_WIDTH:	// FONT B
		DotFillToBuf(&esc_sts[current_channel].font_buf.font_cn_b[0][0], FONT_CN_B_WIDTH, FONT_CN_B_HEIGHT, 1);
		break;
	}
}
#endif
extern void GetEnglishHRIFont(uint8_t ascii)
{
	uint8_t larger;
	larger = esc_sts[current_channel].larger;
	esc_sts[current_channel].larger = 0;
	switch(esc_sts[current_channel].font_en)
	{
	case FONT_A_WIDTH:	// FONT A
#if defined(MP2_BLUETOOTH_DENMARK)
    F25L_Read((ascii)*FONT_A_WIDTH*((FONT_A_HEIGHT+7)/8) + font_en_addr, esc_sts[current_channel].font_buf.font_a[0], sizeof(esc_sts[current_channel].font_buf.font_a));       // get font from sflash
#else

    #if defined(CODEPAGE)
    F25L_Read((ascii)*FONT_A_WIDTH*((FONT_A_HEIGHT+7)/8) + font_en_addr, esc_sts[current_channel].font_buf.font_a[0], sizeof(esc_sts[current_channel].font_buf.font_a));
    #else
	memcpy(esc_sts[current_channel].font_buf.font_a, (const unsigned char*)(FontList[FNT_ASC_12_24].address)+(ascii-0x20)*FontList[FNT_ASC_12_24].size, sizeof(esc_sts[current_channel].font_buf.font_a));
    #endif
#endif
		break;
	case FONT_B_WIDTH:	// FONT B
		memcpy(esc_sts[current_channel].font_buf.font_b, (const unsigned char*)(FontList[FNT_ASC_8_16].address)+(ascii-0x20)*FontList[FNT_ASC_8_16].size, sizeof(esc_sts[current_channel].font_buf.font_b));
        #ifdef ASCII9X24
        LeftRightSwap(esc_sts[current_channel].font_buf.font_b[0], sizeof(esc_sts[current_channel].font_buf.font_b));
        #endif
		break;
	}
	switch(esc_sts[current_channel].font_en)
	{
	case FONT_A_WIDTH:	// FONT A
		DotFillToBuf(&esc_sts[current_channel].font_buf.font_a[0][0], FONT_A_WIDTH, FONT_A_HEIGHT, 0);
		break;
	case FONT_B_WIDTH:	// FONT B
		DotFillToBuf(&esc_sts[current_channel].font_buf.font_b[0][0], FONT_B_WIDTH, FONT_B_HEIGHT, 0);
		break;
	}
	esc_sts[current_channel].larger = larger;
}


