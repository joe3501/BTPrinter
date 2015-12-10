#include "hw_platform.h"
#include <string.h>
#include "basic_fun.h"
#include "Font.h"


uint16_t max_start_col=0;
//======================================================================================================
//@�˺���������΢�Ż�һ�㣬��ӡ������ͺ���Ŵ��ڹ����ӡ����ʱ�Ѿ������ˣ��˴����Բ���Ҫ���ǷŴ�Ĵ���
//��Ȼ�˴��Ĵ���������ʡRAM�ռ䣡����
extern void DotBufFillToPrn(uint8_t *buf, uint16_t max_col, uint16_t max_rowbyte, uint16_t min_row, uint16_t max_row, uint8_t ratio_width, uint8_t ratio_height)
{
 //  DotBufFillToPrn(&CURRENT_ESC_STS.dot[0][0], CURRENT_ESC_STS.start_dot, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), CURRENT_ESC_STS.dot_minrow, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), 1, 1);
	uint16_t row, col, start_col, bit;
	uint8_t bits, mask;
	uint8_t dot[LineDot/8];
	uint8_t ratio;

	if (max_col*ratio_width > LineDot) return;	// ��������

	// ���뷽ʽ����
	switch (CURRENT_ESC_STS.align)
	{
	case 0:		// left align,�����
	default:
		start_col = 0;
		break;
	case 1:		// middle align�����ж���
		start_col = (LineDot - max_col*ratio_width) >> 1;
		break;
	case 2:		// right align���Ҷ���
		start_col = LineDot - max_col*ratio_width;
		break;
	}
	// ���µ��õ���
#if defined(TM_T88II) || defined(SW40)
	if (CURRENT_ESC_STS.upside_down)
#else
	if (0)
#endif
	{       //����
		for (row=max_row-1; row >= min_row; row--)
		{
			bits = 0;
			do
			{
				mask = (1 << bits);
				MEMSET(dot, 0, sizeof(dot));
				for (col=0; col<max_col; col++)
				{
					if (buf[col*max_rowbyte+row] & mask)
					{
						bit = (LineDot - 1) - (start_col + col*ratio_width);
						for (ratio=0; ratio<ratio_width; ratio++, bit--)
						{
							dot[bit >> 3] |= 1<<(7-(bit & 0x07));
						}
					}
				}
				for (ratio=0; ratio<ratio_height; ratio++)
				{
                   if (clr_all_dot==1)
                       break;
					TPPrintLine(dot);
				}

                if (clr_all_dot==1)
                     break;
			}
			while ((++bits)<8);
		}
	}
	else      //�����ӡ
	{

		for (row=min_row; row<max_row; row++)        //������ת��Ϊ����
		{
			bits = 7;
			do
			{
				mask = (1 << bits);
				MEMSET(dot, 0, sizeof(dot));
				for (col=0; col<max_col; col++)
				{
					if (buf[col*max_rowbyte+row] & mask)
					{
						bit = start_col + col*ratio_width;       //��ʼ����λ��+������*��Ŵ���
						for (ratio=0; ratio<ratio_width; ratio++, bit++)    //ʵ�ֺ���Ŵ�
						{
							dot[bit >> 3] |= 1<<(7-(bit & 0x07));
						}
					}
				}
				for (ratio=0; ratio<ratio_height; ratio++)          //ʵ������Ŵ�
				{
                  if (clr_all_dot==1)
                      break;
					TPPrintLine(dot);
				}
                if (clr_all_dot==1)
                     break;
			}
			while (bits--);
		}
	}
}
//======================================================================================================
extern void BufFillToPrn(uint16_t n)
{
	uint16_t row;

    if (clr_all_dot == 1)
    {
        clr_all_dot = 0;
    }

	if (CURRENT_ESC_STS.start_dot)
	{
		DotBufFillToPrn(&CURRENT_ESC_STS.dot[0][0], max_start_col, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), CURRENT_ESC_STS.dot_minrow, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), 1, 1);
	}
	// �м�����
	row = (ARRAY_SIZE(CURRENT_ESC_STS.dot[0]) - CURRENT_ESC_STS.dot_minrow) << 3;
	if (row < CURRENT_ESC_STS.linespace)//����߶�С���м�࣬���ӡ��ʣ��Ŀհ���
	{
		n += CURRENT_ESC_STS.linespace-row;
	}
	if (n)
	{
		TPFeedLine(n);
	}
}

extern void BufFillToPrn_0(uint16_t n)
{

    if (clr_all_dot == 1)
    {
        clr_all_dot = 0;
    }

	if (CURRENT_ESC_STS.start_dot)
	{
		DotBufFillToPrn(&CURRENT_ESC_STS.dot[0][0], max_start_col, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), CURRENT_ESC_STS.dot_minrow, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), 1, 1);
	}

	if (n)
	{
		TPFeedLine(n);
	}

}
//======================================================================================================
extern void PrintCurrentBuffer(uint16_t n)
{
	// �Ƚ�ԭ�ȵ����ݴ�ӡ����
	BufFillToPrn(n);
	// Ȼ����ջ�����
	CURRENT_ESC_STS.bitmap_flag = 0;
	MEMSET(CURRENT_ESC_STS.dot, 0, sizeof(CURRENT_ESC_STS.dot));
	CURRENT_ESC_STS.start_dot = 0;
    max_start_col =0;
	CURRENT_ESC_STS.dot_minrow = ARRAY_SIZE(CURRENT_ESC_STS.dot[0]);
}

extern void PrintCurrentBuffer_0(uint16_t n)
{
	// �Ƚ�ԭ�ȵ����ݴ�ӡ����
	BufFillToPrn_0(n);
	// Ȼ����ջ�����
	CURRENT_ESC_STS.bitmap_flag = 0;
	MEMSET(CURRENT_ESC_STS.dot, 0, sizeof(CURRENT_ESC_STS.dot));
	CURRENT_ESC_STS.start_dot = 0;
    max_start_col =0;
	CURRENT_ESC_STS.dot_minrow = ARRAY_SIZE(CURRENT_ESC_STS.dot[0]);
}
//======================================================================================================
extern void DotFillToBuf(uint8_t *buf, uint16_t col, uint8_t row, uint8_t underline)
{
	uint16_t x, width, maxwidth;
	uint8_t nrow, ncol;
	uint16_t start_dot;
	uint8_t start_row;

    uint8_t dot[FONT_CN_A_HEIGHT*2/8]; 	// �Ŵ�2��////////////////////////////dot[24*2/8]=dot[6]

	CURRENT_ESC_STS.bitmap_flag = 1;
	row >>= 3;		// �߶���λת��Ϊ�ֽ�,�������λ��

	nrow = row * ((CURRENT_ESC_STS.larger & 0x0f)+1);                    //�Ŵ��ĸ߶ȣ�largerǰ4λ�Ÿ߷Ŵ�������4λ�ſ�Ŵ���
	if (nrow > ARRAY_SIZE(CURRENT_ESC_STS.dot[0]))                     //�Ŵ��ĸ߶�>���߶�(����߶�)
	{
		return;
	}
	start_row = ARRAY_SIZE(CURRENT_ESC_STS.dot[0])-nrow;            //
	// ��ǰ�Ļ�����û�пռ��������µĴ�ӡ�ַ�
	ncol = col*(((CURRENT_ESC_STS.larger >> 4)& 0x0f) + 1);                         //CURRENT_ESC_STS.larger >> 4 ȡ����Ŵ���,�Ŵ��Ŀ��
	maxwidth = CURRENT_ESC_STS.leftspace + CURRENT_ESC_STS.print_width;
	if (maxwidth > LineDot)
	{
		maxwidth = LineDot - CURRENT_ESC_STS.leftspace;
	}
	if (CURRENT_ESC_STS.start_dot && ((CURRENT_ESC_STS.start_dot + ncol) > (CURRENT_ESC_STS.leftspace + maxwidth)))////��ǰ�ռ䲻�㣬��ӡ������
	{
		PrintCurrentBuffer(0);
	}
	if (CURRENT_ESC_STS.start_dot == 0)
	{
		if ((ncol > maxwidth) && (ncol > (LineDot - CURRENT_ESC_STS.leftspace)))
		{                                             //����Ȳ����ó�λ��������߾�Ļ�
			CURRENT_ESC_STS.start_dot = LineDot - ncol;
		}
		else//��������࿪ʼ��ӡ
		{
			CURRENT_ESC_STS.start_dot = CURRENT_ESC_STS.leftspace;
		}
	}

	start_dot = CURRENT_ESC_STS.start_dot;//��¼Ҫ���»��ߵ���ʼλ��

	for (x=0; x<col; x++)                 //����Ŵ�
	{
		FontEnlarge(dot,buf, row);//����Ŵ󣬱��
		buf+= row;                        //ָ���������ֽڴ�С�����ܵ�����һ������
		width = 0;
		do
		{
			if (CURRENT_ESC_STS.start_dot < ARRAY_SIZE(CURRENT_ESC_STS.dot))
			{
				MEMCPY(&CURRENT_ESC_STS.dot[CURRENT_ESC_STS.start_dot][start_row], dot, nrow);
				CURRENT_ESC_STS.start_dot++;
			}
			else
			{
				break;
			}
		}
		while (width++ < (CURRENT_ESC_STS.larger >> 4));//CURRENT_ESC_STS.larger����λ�洢�Ŵ�߶�
	}
	if (underline) FontUnderline(start_dot, CURRENT_ESC_STS.start_dot);

	if (start_row < CURRENT_ESC_STS.dot_minrow)
	{
		CURRENT_ESC_STS.dot_minrow = start_row;
	}

	// �����ǰ�Ļ��������������ּ�࣬������ּ�࣬������ܣ����ӡ����
	if ((CURRENT_ESC_STS.start_dot + CURRENT_ESC_STS.charspace) > (CURRENT_ESC_STS.leftspace + maxwidth))
	{

		PrintCurrentBuffer(0);
	}
	else
	{

		// �ַ�������
		CURRENT_ESC_STS.start_dot += CURRENT_ESC_STS.charspace;

	}
	if (CURRENT_ESC_STS.revert)//��෴��
    {
        for (x=(start_dot+ncol); x<CURRENT_ESC_STS.start_dot; x++)       //col Ϊwidth
        {
            MEMSET(&CURRENT_ESC_STS.dot[x][start_row], 0xff, nrow);
        }
    }

	if (underline) FontUnderline(start_dot, CURRENT_ESC_STS.start_dot);

    if(CURRENT_ESC_STS.start_dot > max_start_col)
	{
		max_start_col = CURRENT_ESC_STS.start_dot;
	}
}
//======================================================================================================
extern void  PictureDotFillToBuf(uint8_t *buf, uint16_t col, uint16_t row)
{
    uint16_t x,start_row;


    row >>=3;
    if(row > ARRAY_SIZE(CURRENT_ESC_STS.dot[0]))
	{
		row = ARRAY_SIZE(CURRENT_ESC_STS.dot[0]);
	}

	start_row = ARRAY_SIZE(CURRENT_ESC_STS.dot[0])-row;

    if(start_row < CURRENT_ESC_STS.dot_minrow)
	{
		CURRENT_ESC_STS.dot_minrow = start_row;
	}
	if (CURRENT_ESC_STS.start_dot == 0)
	{
		CURRENT_ESC_STS.start_dot = CURRENT_ESC_STS.leftspace;
	}
    for(x=0; x<col; x++)
	{
		buf += row;
		if(CURRENT_ESC_STS.start_dot < ARRAY_SIZE(CURRENT_ESC_STS.dot))
		{
			MEMCPY(&CURRENT_ESC_STS.dot[CURRENT_ESC_STS.start_dot][start_row], buf, row);
			CURRENT_ESC_STS.start_dot++;
		}
		else
		{
			break;
		}

	}

    if(CURRENT_ESC_STS.start_dot)
	{
		DotBufFillToPrn(&CURRENT_ESC_STS.dot[0][0], CURRENT_ESC_STS.start_dot, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), CURRENT_ESC_STS.dot_minrow, ARRAY_SIZE(CURRENT_ESC_STS.dot[0]), 1, 1);
  //       printf("DOT\n");

	}
	// Ȼ����ջ�����
	MEMSET(CURRENT_ESC_STS.dot, 0, sizeof(CURRENT_ESC_STS.dot));
	CURRENT_ESC_STS.start_dot = 0;
	CURRENT_ESC_STS.dot_minrow = ARRAY_SIZE(CURRENT_ESC_STS.dot[0]);


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


