/**
* @file ESC_POS_test.c
* @brief ����ESC/POSָ��ļ�����
* @version V0.0.1
* @author kent.zhou
* @date 2015��12��4��
* @copy
*
* �˴���Ϊ���ںϽܵ������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*
*/
#ifdef DEBUG_VER
#include "Type.h"
#include "ESC_POS_test.h"
#include "TP.h"
#include "pic.h"
#include "esc_p.h"
#include "res_spi.h"

extern unsigned int					dwPictureLBA;
/*
 * @brief: ����ESC��ʼ�����
 */
void ESC_POS_test_esc(void)
{

    uint32_t len,i;
    char buf[64];

	//debug_cnt = 0;
	current_channel = 0;
     PrintBufToZero();
    len = snprintf(buf, sizeof(buf),  "\n");
    TPPrintAsciiLine(buf,len);
    TPPrintAsciiLine(buf,len);

	//ESC @  ��ʼ����ӡ��,����Ĭ�����ô�ӡһ������
    len = snprintf(buf, sizeof(buf), "\x1b@1234567890����\n");
    TPPrintAsciiLine(buf,len);

	//ESC SP n -- �ı����м��  ESC ! n -- �޸Ĵ�ӡģʽ
	len = snprintf(buf, sizeof(buf), "\x1b\x20\x10\x1b!\xff 1234567890����\n");
    TPPrintAsciiLine(buf,len);

	//ESC $ nl nh  ���þ��Դ�ӡλ�� //ESC - n	ѡ��/ȡ���»���ģʽ  �Ͱ��ֽڵĵ�2λ��ʾ�»��߿��  0�� ���ı�  1��һ����   2:2����
								    //								 �߰��ֽڵĵ�2λ��ʾ�Ƿ���Ҫ�»���  0�� ȡ��  else: ѡ��
	len = snprintf(buf, sizeof(buf), "\x1b@\x1b$\x20\x01\x1b-\x12 1234567890����\n");
    TPPrintAsciiLine(buf,len);

	//ESC 3 n  �����м��   //ESC J n ��ӡ����ֽ
	len = snprintf(buf, sizeof(buf), "\x1b@\x1b\x33\x40 123456\x1bJ\x05 7890����\n");
    TPPrintAsciiLine(buf,len);

	//ESC 2  ����Ĭ���м��   //ESC a n  ѡ����뷽ʽ
	len = snprintf(buf, sizeof(buf), "\x1b\x32\x1b\x61\x02 1234567890����\n");
    TPPrintAsciiLine(buf,len);
	len = snprintf(buf, sizeof(buf), "\x1b\x61\x01 1234567890����\n");
    TPPrintAsciiLine(buf,len);
	len = snprintf(buf, sizeof(buf), "\x1b\x61\x30 1234567890����\n");
    TPPrintAsciiLine(buf,len);

	////ESC d n ��ӡ����ǰ��ֽn �� //ESC { n ��/�رյߵ���ӡģʽ
	len = snprintf(buf, sizeof(buf), "\x1b{\x01 1212345\x1b\x64\x08\x1b{\x30 67890����\n");
    TPPrintAsciiLine(buf,len);

	//ESC M n
	len = snprintf(buf, sizeof(buf), "\x1bM\x01 121234567890����\n\x1b@");
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "\n\n\n\n\n");
    TPPrintAsciiLine(buf,len);
}


static unsigned char	line_buffer[LineDot/8];
static unsigned char	line_offset;
extern uint8_t const Byte2DotNumTbl[];
void print_CharLine(void)
{
	unsigned char	tmp[2*LineDot+4];
	unsigned int	i,j,k,cnt,pos;
	
	for(i = 0,cnt = 0; i < LineDot/8;i++)
	{
		cnt +=  Byte2DotNumTbl[line_buffer[i]];
	}

	tmp[0] = 0x1b;
	tmp[1] = 0x27;

	tmp[2] = (cnt&0xff);
	tmp[3] = (cnt>>8)&0xff;

	for(i = 0,k = 4; i < LineDot/8;i++)
	{
		for(j=0; j<8; j++)
		{
			if(line_buffer[i] & 0x80)
			{
				pos = CURRENT_ESC_STS.leftspace+i*8+j;
				tmp[k++]=(pos&0xff);
				tmp[k++]=(pos>>8)&0xff;
			}
			line_buffer[i] <<= 1;
		}	
	}
	TPPrintAsciiLine(tmp,k);
}

/**
* @brief ����ɫ��BMPͼƬ��ӡ����
*/
static void print_PictureStreem(unsigned int dwLBA, unsigned int offset)
{
	unsigned char					*pData, *pOrg;
	unsigned short					w, h;
	int								i, j;
	//unsigned short					lcd_x,lcd_y;
	unsigned int					data_offset,remain;
	unsigned char					tmp[26];


	//lcd_x							= CURRENT_ESC_STS.leftspace;
	//lcd_y							= y;

	// pOrg   res_buffer��ʼ��ַ
	// pData  ����ָ��
	pOrg							= read_resdata(dwLBA);

	if(pOrg == (unsigned char*)0)
		return;

	pData						= pOrg + offset;

	if ((pData+26) >= (pOrg + 0x200))
	{
		remain = pOrg + 0x200 - pData;
		MEMCPY(tmp,pData,remain);

		dwLBA++;
		pOrg							= read_resdata(dwLBA);
		pData = pOrg;
		MEMCPY(tmp+remain,pData,26-remain);

		if ((*tmp != 'B')||(*(tmp+1) != 'M'))
		{
			return;		//��ЩBMPͼƬ�ļ��ı�־�ֽڲ�����BM
		}

		data_offset = *(unsigned short*)(tmp+10);

		w							= *(unsigned int*)(tmp+18);
		h							= *(unsigned int*)(tmp+22);

		pData						+= data_offset - remain;
	}

	else
	{
		if ((*pData != 'B')||(*(pData+1) != 'M'))
		{
			return;		//��ЩBMPͼƬ�ļ��ı�־�ֽڲ�����BM
		}

		data_offset = *(unsigned short*)(pData+10);	//������λͼ���ݵ���ʼ��ַ

		w							= *(unsigned int*)(pData+18);
		h							= *(unsigned int*)(pData+22);

		pData						+= data_offset;
	}

	pOrg						+= 0x200;
	if (pData > pOrg)
	{
		data_offset = pData - pOrg;
		dwLBA ++;
		pOrg							= read_resdata(dwLBA);
		pData = pOrg + data_offset;
		pOrg += 0x200;
	}

	i							= w;
	//j							= lcd_x;

	for(;h>0; h--)
	{
		line_offset = 0;
		MEMSET(line_buffer,0,LineDot/8);
		while(w>0)
		{
lop:
			//print_PushChar(*pData);
			if(line_offset <  LineDot/8)
			{
				line_buffer[line_offset] = *pData;
				line_offset++;
			}

			pData				++;
			w -= 8;
			if(pData >= pOrg)
			{
				dwLBA++;
				pData			= read_resdata(dwLBA);
				goto lop;
			}
		}
		//lcd_y++;
		w						= i;		// �ָ����
		if((w/8)%4)
		{
			pData += 4 - ((w/8)%4);		//����BMPһ��������4�ֽڶ��룬������Ҫ����  
		}
		
		print_CharLine();
	}
}

/**
* @brief ����ͼƬID����ָ��λ������ʾͼƬ
* @param[in] int x,y ���������
* @param[in] unsigned int PicID ͼƬID
*/
void print_PictureOut(unsigned int PicID)
{
	unsigned int					*pIDArray;
	unsigned int					pic_address, pic_sector;
	//volatile int	temp;


	if(dwPictureLBA == 0)
	{
		// ͼƬװ��ʧ��
		return;
	}

	dwPictureLBA +=1;
	// װ��ID����,ÿ��IDռ4�ֽ�,����ͨ����ַdwPictureLBA,����ID��ƫ�Ƶõ���ID��ַ
	// ���ڵ�����
	pIDArray						= (unsigned int*)read_resdata(dwPictureLBA + (PicID*4)/512);
	if(pIDArray)
	{
		pIDArray					+= PicID % 128;
	}

	// ȡ�õ�ַ
	pic_address						= *pIDArray;
	if (pic_address)
	{
		pic_sector						= dwPictureLBA + (pic_address / 512);	// �������ڵ�
		pic_address						&= (512-1);								// �����ڵ�ƫ��ֵ

		print_PictureStreem(pic_sector, pic_address);
	}
}

/*
 * @brief: ����ESC��ӡ��������,ͨ���������ӡһ��BMPͼ��
 */
void ESC_POS_test_esc_special(void)
{
	current_channel = 0;
     PrintBufToZero();


	//ESC ��ml mh l1 h1 l2 h2 l3 h3 �� li hi��  ��ӡ����
			//ml+mh*256��ʾ��һ����Ҫ��ӡ�ĵ�����������ģ�li+hi*256���Ǵ�������Ҫ��ӡ�ĵ��λ��
			//ʵ����ͨ����ָ����Դ�ӡλͼ

	print_PictureOut(PIC_LOGO);
}
#endif