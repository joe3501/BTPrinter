/**
* @file ESC_POS_test.c
* @brief 测试ESC/POS指令集的兼容性
* @version V0.0.1
* @author kent.zhou
* @date 2015年12月4日
* @copy
*
* 此代码为深圳合杰电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
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
 * @brief: 测试ESC开始的命令集
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

	//ESC @  初始化打印机,先用默认设置打印一行数据
    len = snprintf(buf, sizeof(buf), "\x1b@1234567890测试\n");
    TPPrintAsciiLine(buf,len);

	//ESC SP n -- 改变右行间距  ESC ! n -- 修改打印模式
	len = snprintf(buf, sizeof(buf), "\x1b\x20\x10\x1b!\xff 1234567890测试\n");
    TPPrintAsciiLine(buf,len);

	//ESC $ nl nh  设置绝对打印位置 //ESC - n	选择/取消下划线模式  低半字节的低2位表示下划线宽度  0： 不改变  1：一点行   2:2点行
								    //								 高半字节的低2位表示是否需要下划线  0： 取消  else: 选择
	len = snprintf(buf, sizeof(buf), "\x1b@\x1b$\x20\x01\x1b-\x12 1234567890测试\n");
    TPPrintAsciiLine(buf,len);

	//ESC 3 n  设置行间距   //ESC J n 打印并走纸
	len = snprintf(buf, sizeof(buf), "\x1b@\x1b\x33\x40 123456\x1bJ\x05 7890测试\n");
    TPPrintAsciiLine(buf,len);

	//ESC 2  设置默认行间距   //ESC a n  选择对齐方式
	len = snprintf(buf, sizeof(buf), "\x1b\x32\x1b\x61\x02 1234567890测试\n");
    TPPrintAsciiLine(buf,len);
	len = snprintf(buf, sizeof(buf), "\x1b\x61\x01 1234567890测试\n");
    TPPrintAsciiLine(buf,len);
	len = snprintf(buf, sizeof(buf), "\x1b\x61\x30 1234567890测试\n");
    TPPrintAsciiLine(buf,len);

	////ESC d n 打印并向前走纸n 行 //ESC { n 打开/关闭颠倒打印模式
	len = snprintf(buf, sizeof(buf), "\x1b{\x01 1212345\x1b\x64\x08\x1b{\x30 67890测试\n");
    TPPrintAsciiLine(buf,len);

	//ESC M n
	len = snprintf(buf, sizeof(buf), "\x1bM\x01 121234567890测试\n\x1b@");
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
* @brief 将单色的BMP图片打印出来
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

	// pOrg   res_buffer起始地址
	// pData  工作指针
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
			return;		//有些BMP图片文件的标志字节并不是BM
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
			return;		//有些BMP图片文件的标志字节并不是BM
		}

		data_offset = *(unsigned short*)(pData+10);	//真正的位图数据的起始地址

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
		w						= i;		// 恢复宽度
		if((w/8)%4)
		{
			pData += 4 - ((w/8)%4);		//由于BMP一行数据是4字节对齐，所以需要处理  
		}
		
		print_CharLine();
	}
}

/**
* @brief 根据图片ID，在指定位置上显示图片
* @param[in] int x,y 输出的坐标
* @param[in] unsigned int PicID 图片ID
*/
void print_PictureOut(unsigned int PicID)
{
	unsigned int					*pIDArray;
	unsigned int					pic_address, pic_sector;
	//volatile int	temp;


	if(dwPictureLBA == 0)
	{
		// 图片装载失败
		return;
	}

	dwPictureLBA +=1;
	// 装入ID数组,每个ID占4字节,所以通过基址dwPictureLBA,加上ID的偏移得到该ID地址
	// 所在的扇区
	pIDArray						= (unsigned int*)read_resdata(dwPictureLBA + (PicID*4)/512);
	if(pIDArray)
	{
		pIDArray					+= PicID % 128;
	}

	// 取得地址
	pic_address						= *pIDArray;
	if (pic_address)
	{
		pic_sector						= dwPictureLBA + (pic_address / 512);	// 数据所在的
		pic_address						&= (512-1);								// 扇区内的偏移值

		print_PictureStreem(pic_sector, pic_address);
	}
}

/*
 * @brief: 测试ESC打印曲线命令,通过此命令打印一个BMP图案
 */
void ESC_POS_test_esc_special(void)
{
	current_channel = 0;
     PrintBufToZero();


	//ESC ’ml mh l1 h1 l2 h2 l3 h3 … li hi…  打印曲线
			//ml+mh*256表示这一行需要打印的点数，后面跟的（li+hi*256）是此行内需要打印的点的位置
			//实际上通过此指令可以打印位图

	print_PictureOut(PIC_LOGO);
}
#endif