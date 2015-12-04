

/**
* @file res_spi.c
* @brief ˢ������Դ����ģ��,���ڹ����ֿ⣬ͼƬ���洢�����ȹ���
*        ��ģ�齫�ֿ����Դ�洢��SPI FLASH��
* @version V0.0.2
* @author joe
* @date 2010��2��5��
* @note
*     �޸ģ�
*			������Ŀ������Ҫ֧��GBK�ֿ⣬����������һ��12*12��GB2312�ֿ⣬SPI FLASH�������������Խ�12*12��
*			��GB2312�ֿ�ŵ�MCU FLASH�ڲ�ȥ�ˡ�
*
* @version V0.0.1
* @author zhongyh
* @date 2009��9��4��
* @note
*     ÿ�ֲ�ͬ����Դ����һ��������BPT�У�����BPT_DataType������
*
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x_lib.h"
#include "res_spi.h"
#include "ff.h"
#include "spi_flash.h"
#include "Esc_p.h"
#include "font8x16.h"
#include "assert.h"

/**
*	@brief	��Դ�ļ���Ϣ�ṹ��
*/
typedef struct  
{
	unsigned char					magic[4];
	unsigned int					xor;
	unsigned int					xor_data;
	unsigned int					length;
	unsigned char					OEMName[16];
	unsigned char					Version[16];
	unsigned char					Date[16];
}TPackHeader;

/**
*	@brief	ÿһ����Դ����Ϣ�ṹ��
*/
#pragma pack(1)
typedef struct {
	unsigned char				RIT_OEMName[16];    		// 0  
	unsigned char				RIT_ManName[16];    		// 10 
	unsigned char				RIT_ModuleName[16];			// 20 
	unsigned short				RIT_ProdID;					// 30 ��ƷID
	unsigned short				RIT_CodeVersion;    		// 32 ��������汾
	unsigned short				RIT_Date;            		// 34 ����ʱ��
	unsigned char				RIT_SN[10];            		// 36 ���к�
	unsigned int				RIT_TotSect;        		// 40 ��������ռ��������
	unsigned int				RIT_CodeSec;        		// 44 ��������ռ��������
	unsigned char				RIT_VerifyType;        		// 48 ��������У�鷽ʽ
	unsigned char				RIT_Endian;        			// 49 CPU Core
	unsigned short				RIT_DataType;         		// 4A �˷����ݵ����ͣ���������������룬��ָ�����������ͣ�
	unsigned int				RIT_StartAddr;        		// 4C ����������ram�д�ŵ���ʼ��ַ
	unsigned int				RIT_RunAddr;        		// 50 ���д���ʱPC��ת�ĵ�ַ
	unsigned char   			RIT_DataID;					// 54 �˷���Դ��ID��
	unsigned char				RIT_Reserved0;				// 55
	unsigned char				RIT_Align;					// 56 �˷���Դ����С���뵥λ��Ϊ2^n�ֽ�
	unsigned char				RIT_Reserved1;				// 57
	unsigned char				RIT_StrVer[13];				// 58  ��Դ�İ汾�ţ���ʽΪ20100610_0001
	unsigned char				RIT_Reserved[0x18B];		// 65 
	unsigned int				RIT_DataCRC32;				// 1F0 ��������CRC32��������һ�������𣬵�RIT_CodeSecֹ���������ݵ�CRC32ֵ��CRC���������xml�ļ��е����������ɡ�
	unsigned int				RIT_Next;            		// 1F4 ��һ��RIT�ڴ����еľ��Ե�ַ
	unsigned int				RIT_CheckSum;        		// 1F8 �˷���Դ��RIT��CRC32У��ֵ���� 
	unsigned int				RIT_TrailSig;       		// 1FC ������־(0x00,0x00,0x55,0xAB)
}TResInfoTable;

#pragma pack()

#define MAX_FNT_CNT				FNT_CHN_24_24

/**
*	@brief  ˢ������Ŀ���õ���������Ϣ��
*/
TFontInfo FontList[MAX_FNT_CNT+1] = {
	{	0, 8,  16, 16, 0	},							// ascii 8x16
	{	0, 12, 24, 36, 0	},							// ascii 12x24
	{	0, 16, 16, 32, 0	},							// GBK 16x16
	{	0, 24, 24, 72, 0	},							// GBK 24x24
};


unsigned int					dwPictureLBA;
static unsigned char			res_buffer[512];

static FIL						resource_file;		//��Դ�ļ�ר��һ���ļ����
FATFS							fs;
#define res_file				"/resdata.bin"

/**
* @def ÿһ����Դ������BPT�У�BPT_DataType��Ӧ��ֵ
*/
#define RESTYPE_CODE		0x80					// ϵͳ��������
#define RESTYPE_FNT			0x10					// �ֿ���Դ��
#define RESTYPE_PIC			0x11					// ͼƬ��Դ��

/**
* @brief ��ָ����ַ��ʼ����BPT��������Ƿ���ͬ
*/
static int res_loadres(unsigned int dwLBA, unsigned char *pType, unsigned char *pID)
{

	TResInfoTable			*pResTable;
	unsigned int			rd;
	
	if (f_lseek(&resource_file,dwLBA*512) != FR_OK)
	{
		return -1;
	}

	if (f_read(&resource_file,res_buffer,512,&rd) != FR_OK)
	{
		return -1;
	}

	
	pResTable	= (TResInfoTable*)res_buffer;
	
	if( res_buffer[510] != 0x55 ||
		res_buffer[511] != 0xAB)
	{
		return -1;
	}
	
	*pType					= pResTable->RIT_DataType;
	*pID					= pResTable->RIT_DataID;

	return 0;
}
/**
* @brief ��װ�����ֿ�
*/
int setup_font(unsigned int dwLBA, unsigned char id)
{
	if( id == FNT_CHN_24_24 || id == FNT_CHN_16_16)
	{
		FontList[id].address = dwLBA;
	}
	return 0;
}

/**
* @brief �ҵ���ͼƬ��Դ������ID���õ���ַ
*/
void setup_picture(unsigned int dwLBA)
{
	dwPictureLBA					= dwLBA;
}

/**
* @brief ��Դģ���ʼ��
* @return 0:�ɹ���ʼ��
*        -1:��ʼ��SD��ʧ��
*        -2:SPI Flash���Ҳ�����Դ
*/
int res_init(void)
{

	unsigned int					dwLBA;
	unsigned char					res_type, res_id;
	unsigned int					rd;
	FRESULT	ret;

	FontList[FNT_ASC_8_16].address	= (unsigned int)fontb_en;
	FontList[FNT_ASC_12_24].address	= (unsigned int)fonta_en;
	FontList[FNT_CHN_16_16].address	= 0;
	FontList[FNT_CHN_24_24].address	= 0;
	dwPictureLBA					= 0;


	//�����ļ�ϵͳ
	f_mount(0, &fs);										// װ���ļ�ϵͳ

	//load_logo_res();
re_open:	
	ret = f_open(&resource_file, res_file, FA_OPEN_EXISTING | FA_READ );
	if(ret  != FR_OK )
	{
		if (ret == FR_NO_FILESYSTEM)
		{
			ret = f_mkfs(0,1,512);
			if (ret != FR_OK)
			{
				return -1;
			}
			else
			{
				goto re_open;
			}
		}
		else
		{
			return -1;
		}
	}

	f_lseek(&resource_file,0);

	// ==========================================================================================================
	// 1. У����Դ�ļ�
	if( f_read(&resource_file, res_buffer, 512, &rd) != FR_OK )
	{
		f_close(&resource_file);
		return -2;
	}


	// У���ļ�ͷ
	if( res_buffer[0] != 'J' || res_buffer[1] != 'B' || res_buffer[2] != 'L' || res_buffer[3] != '3' )
	{
		f_close(&resource_file);
		return -2;
	}
	
	// װ����Դ
	dwLBA							= 0;
	do
	{
		// װ��һ����
		dwLBA += 1;
		if( res_loadres(dwLBA, &res_type, &res_id) != 0)
		{
			f_close(&resource_file);
			return -2;
		}
		
		// �ж���Դ����
		if(RESTYPE_FNT == res_type)
		{
			setup_font(dwLBA+1, res_id);		// ��װ����
		}
		else if(RESTYPE_PIC == res_type)
		{
			setup_picture(dwLBA + 1);
		}
		else
		{
			//return -2;
		}
		
		// ȡ��һ����Դ��ַ
		dwLBA						= *(unsigned int*)&res_buffer[0x1F4];
	}while(dwLBA != 0xFFFFFFFF);

    return 0;
}

/**
* @brief ��SPI FLASH�ж�ȡһ����������Դ��������res_buffer��
* @param[in] unsigned int dwLBAҪ��ȡ����Դ��ַ
* @return unsigned char *���ݴ���ڵ�ַ
*/
unsigned char *read_resdata(unsigned int dwLBA)
{
	unsigned int	rd;
	unsigned int	retry_cnt = 2; 

	if (resource_file.flag & FA__ERROR)
	{
		resource_file.flag &= ~FA__ERROR;
	}

	while(retry_cnt)
	{
		if (f_lseek(&resource_file,dwLBA*512) != FR_OK)
		{
			retry_cnt--;
			f_mount(0,&fs);
			//f_mount(0,&fs);
			if(f_open(&resource_file,res_file,FA_OPEN_EXISTING | FA_READ) != FR_OK)
			{
				return (unsigned char *) 0; 
			}
			else
			{
				continue;
			}
		}

		if (f_read(&resource_file,res_buffer,512,&rd) != FR_OK)
		{
			retry_cnt--;
			f_mount(0,&fs);
			//f_mount(0,&fs);
			if(f_open(&resource_file,res_file,FA_OPEN_EXISTING | FA_READ) != FR_OK)
			{
				return (unsigned char *) 0; 
			}
			else
			{
				continue;
			}
		}

		return res_buffer;
	}

	return (unsigned char *) 0;
}


/**
* @brief ����Դ�ļ��л�ȡһ���ַ�����ģ����
* @param[in] unsigned char font_type		��������	
* @param[in] unsigned char *c				�ַ�����	
* @param[in] unsigned char *pBuf			������ģ���ݵĻ�����	
* @param[in] unsigned int size				��ģ���ݵĴ�С	
*/
void font_data_read(unsigned char font_type,unsigned char *c,unsigned char *pBuf,unsigned int size)
{
	//int						i;
	TFontInfo				*pInfo;
	int						offset,len1,len2;
	unsigned char			*pData;
	//unsigned char			*pOrg;
	//if ((font_type != FNT_CHN_16_16)&&(font_type != FNT_CHN_24_24))
	//{
	//	return;
	//}
	assert(font_type == FNT_CHN_16_16 || font_type == FNT_CHN_24_24);

	//if(gFontSize == 12)
	{

		/*
		GBK����ֿ���ƫ�����ļ��㹫ʽΪ��

		GBKindex = ((unsigned char)GBKword[0]-129)*190 +

		((unsigned char)GBKword[1]-64) - (unsigned char)GBKword[1]/128;

		*/
		//offset				= ((c[0] - 0xA1) & 0x7F) * 94 + ((c[1]-0xA1)&0x7F);		//GB2312�ַ���ƫ�Ƽ��㹫ʽ

		offset				= (c[0] - 129) * 190 + (c[1]-64) - (c[1]/128);				//GBK�ַ���ƫ�Ƽ��㹫ʽ
		pInfo				= &FontList[font_type];


		if(pInfo->address == 0)
			return;

		offset				*= pInfo->size;
		pData				= read_resdata(pInfo->address + (offset/512));	// �������ڵ�����
		pData				+= offset & 0x1FF;								// ���������ڵ�ƫ��

		if (((offset&0x1ff)+size) > 512)
		{
			len1 = 512-(offset&0x1ff);
			memcpy(pBuf,pData,len1);
			len2 = size - len1;
			pData = read_resdata(pInfo->address + (offset/512) + 1);
			memcpy(pBuf+len1,pData,len2);
		}
		else
		{
			memcpy(pBuf,pData,size);
		}
	}//12

	//memset(pBuf,0x01,size);

}
