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
 //  DotBufFillToPrn(&esc_sts[current_channel].dot[0][0], esc_sts[current_channel].start_dot, ARRAY_SIZE(esc_sts[current_channel].dot[0]), esc_sts[current_channel].dot_minrow, ARRAY_SIZE(esc_sts[current_channel].dot[0]), 1, 1);
	uint16_t row, col, start_col, bit;
	uint8_t bits, mask;
	uint8_t dot[LineDot/8];
	uint8_t ratio;

	if (max_col*ratio_width > LineDot) return;	// ��������

	// ���뷽ʽ����
	switch (esc_sts[current_channel].align)
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
	if (esc_sts[current_channel].upside_down)
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
				memset(dot, 0, sizeof(dot));
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
				memset(dot, 0, sizeof(dot));
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

	if (esc_sts[current_channel].start_dot)
	{
		DotBufFillToPrn(&esc_sts[current_channel].dot[0][0], max_start_col, ARRAY_SIZE(esc_sts[current_channel].dot[0]), esc_sts[current_channel].dot_minrow, ARRAY_SIZE(esc_sts[current_channel].dot[0]), 1, 1);
	}
	// �м�����
	row = (ARRAY_SIZE(esc_sts[current_channel].dot[0]) - esc_sts[current_channel].dot_minrow) << 3;
	if (row < esc_sts[current_channel].linespace)//����߶�С���м�࣬���ӡ��ʣ��Ŀհ���
	{
		n += esc_sts[current_channel].linespace-row;
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

	if (esc_sts[current_channel].start_dot)
	{
		DotBufFillToPrn(&esc_sts[current_channel].dot[0][0], max_start_col, ARRAY_SIZE(esc_sts[current_channel].dot[0]), esc_sts[current_channel].dot_minrow, ARRAY_SIZE(esc_sts[current_channel].dot[0]), 1, 1);
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
	esc_sts[current_channel].bitmap_flag = 0;
	memset(esc_sts[current_channel].dot, 0, sizeof(esc_sts[current_channel].dot));
	esc_sts[current_channel].start_dot = 0;
    max_start_col =0;
	esc_sts[current_channel].dot_minrow = ARRAY_SIZE(esc_sts[current_channel].dot[0]);
}

extern void PrintCurrentBuffer_0(uint16_t n)
{
	// �Ƚ�ԭ�ȵ����ݴ�ӡ����
	BufFillToPrn_0(n);
	// Ȼ����ջ�����
	esc_sts[current_channel].bitmap_flag = 0;
	memset(esc_sts[current_channel].dot, 0, sizeof(esc_sts[current_channel].dot));
	esc_sts[current_channel].start_dot = 0;
    max_start_col =0;
	esc_sts[current_channel].dot_minrow = ARRAY_SIZE(esc_sts[current_channel].dot[0]);
}
//======================================================================================================
extern void DotFillToBuf(uint8_t *buf, uint16_t col, uint8_t row, uint8_t underline)
{
	uint16_t x, width, maxwidth;
	uint8_t nrow, ncol;
	uint16_t start_dot;
	uint8_t start_row;

    uint8_t dot[FONT_CN_A_HEIGHT*2/8]; 	// �Ŵ�2��////////////////////////////dot[24*2/8]=dot[6]

	esc_sts[current_channel].bitmap_flag = 1;
	row >>= 3;		// �߶���λת��Ϊ�ֽ�,�������λ��

	nrow = row * ((esc_sts[current_channel].larger & 0x0f)+1);                    //�Ŵ��ĸ߶ȣ�largerǰ4λ�Ÿ߷Ŵ�������4λ�ſ�Ŵ���
	if (nrow > ARRAY_SIZE(esc_sts[current_channel].dot[0]))                     //�Ŵ��ĸ߶�>���߶�(����߶�)
	{
		return;
	}
	start_row = ARRAY_SIZE(esc_sts[current_channel].dot[0])-nrow;            //
	// ��ǰ�Ļ�����û�пռ��������µĴ�ӡ�ַ�
	ncol = col*(((esc_sts[current_channel].larger >> 4)& 0x0f) + 1);                         //esc_sts[current_channel].larger >> 4 ȡ����Ŵ���,�Ŵ��Ŀ��
	maxwidth = esc_sts[current_channel].leftspace + esc_sts[current_channel].print_width;
	if (maxwidth > LineDot)
	{
		maxwidth = LineDot - esc_sts[current_channel].leftspace;
	}
	if (esc_sts[current_channel].start_dot && ((esc_sts[current_channel].start_dot + ncol) > (esc_sts[current_channel].leftspace + maxwidth)))////��ǰ�ռ䲻�㣬��ӡ������
	{
		PrintCurrentBuffer(0);
	}
	if (esc_sts[current_channel].start_dot == 0)
	{
		if ((ncol > maxwidth) && (ncol > (LineDot - esc_sts[current_channel].leftspace)))
		{                                             //����Ȳ����ó�λ��������߾�Ļ�
			esc_sts[current_channel].start_dot = LineDot - ncol;
		}
		else//��������࿪ʼ��ӡ
		{
			esc_sts[current_channel].start_dot = esc_sts[current_channel].leftspace;
		}
	}

	start_dot = esc_sts[current_channel].start_dot;//��¼Ҫ���»��ߵ���ʼλ��

	for (x=0; x<col; x++)                 //����Ŵ�
	{
		FontEnlarge(dot,buf, row);//����Ŵ󣬱��
		buf+= row;                        //ָ���������ֽڴ�С�����ܵ�����һ������
		width = 0;
		do
		{
			if (esc_sts[current_channel].start_dot < ARRAY_SIZE(esc_sts[current_channel].dot))
			{
				memcpy(&esc_sts[current_channel].dot[esc_sts[current_channel].start_dot][start_row], dot, nrow);
				esc_sts[current_channel].start_dot++;
			}
			else
			{
				break;
			}
		}
		while (width++ < (esc_sts[current_channel].larger >> 4));//esc_sts[current_channel].larger����λ�洢�Ŵ�߶�
	}
	if (underline) FontUnderline(start_dot, esc_sts[current_channel].start_dot);

	if (start_row < esc_sts[current_channel].dot_minrow)
	{
		esc_sts[current_channel].dot_minrow = start_row;
	}

	// �����ǰ�Ļ��������������ּ�࣬������ּ�࣬������ܣ����ӡ����
	if ((esc_sts[current_channel].start_dot + esc_sts[current_channel].charspace) > (esc_sts[current_channel].leftspace + maxwidth))
	{

		PrintCurrentBuffer(0);
	}
	else
	{

		// �ַ�������
		esc_sts[current_channel].start_dot += esc_sts[current_channel].charspace;

	}
	if (esc_sts[current_channel].revert)//��෴��
    {
        for (x=(start_dot+ncol); x<esc_sts[current_channel].start_dot; x++)       //col Ϊwidth
        {
            memset(&esc_sts[current_channel].dot[x][start_row], 0xff, nrow);
        }
    }

	if (underline) FontUnderline(start_dot, esc_sts[current_channel].start_dot);

    if(esc_sts[current_channel].start_dot > max_start_col)
	{
		max_start_col = esc_sts[current_channel].start_dot;
	}
}
//======================================================================================================
extern void  PictureDotFillToBuf(uint8_t *buf, uint16_t col, uint16_t row)
{
    uint16_t x,start_row;


    row >>=3;
    if(row > ARRAY_SIZE(esc_sts[current_channel].dot[0]))
	{
		row = ARRAY_SIZE(esc_sts[current_channel].dot[0]);
	}

	start_row = ARRAY_SIZE(esc_sts[current_channel].dot[0])-row;

    if(start_row < esc_sts[current_channel].dot_minrow)
	{
		esc_sts[current_channel].dot_minrow = start_row;
	}
	if (esc_sts[current_channel].start_dot == 0)
	{
		esc_sts[current_channel].start_dot = esc_sts[current_channel].leftspace;
	}
    for(x=0; x<col; x++)
	{
		buf += row;
		if(esc_sts[current_channel].start_dot < ARRAY_SIZE(esc_sts[current_channel].dot))
		{
			memcpy(&esc_sts[current_channel].dot[esc_sts[current_channel].start_dot][start_row], buf, row);
			esc_sts[current_channel].start_dot++;
		}
		else
		{
			break;
		}

	}

    if(esc_sts[current_channel].start_dot)
	{
		DotBufFillToPrn(&esc_sts[current_channel].dot[0][0], esc_sts[current_channel].start_dot, ARRAY_SIZE(esc_sts[current_channel].dot[0]), esc_sts[current_channel].dot_minrow, ARRAY_SIZE(esc_sts[current_channel].dot[0]), 1, 1);
  //       printf("DOT\n");

	}
	// Ȼ����ջ�����
	memset(esc_sts[current_channel].dot, 0, sizeof(esc_sts[current_channel].dot));
	esc_sts[current_channel].start_dot = 0;
	esc_sts[current_channel].dot_minrow = ARRAY_SIZE(esc_sts[current_channel].dot[0]);


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


