
/**
* @file res_spi_new.c
* @brief BT Printer��Դ����ģ��,���ڹ����ֿ⣬ͼƬ����
*        ��ģ�齫�ֿ⡢ͼƬ����Դ�洢��SPI FLASH�С�
*
* @version V0.0.1
* @author kent.zhou
* @date 2015��12��01��
* @note
*     ÿ�ֲ�ͬ����Դ����һ��������BPT�У�����BPT_DataType������
*
* 
*/
/*
	��ģ����Ҫ�ṩ�Ľӿ��ǣ�1.��ɽ���Դ��FAT������SPI FLASH�Ĺ��ܣ�
							2.�����Դ�İ�װ��Ҳ����˵���ֿ���Դ�������γ�һ�ֶ�Ӧ��ϵ��ͼƬ��Դ��
							��Ҫ�õ���ͼƬID�γ�һ�ֶ�Ӧ��ϵ
*/
/* Private Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ff.h"
#include "stm32f10x_lib.h"
#include "stm32f10x_type.h"
#include "res_spi.h"
#include "spi_flash.h"
#include "Esc_p.h"
#include "crc32.h"
#include "font8x16.h"
#include "spi_flash_config.h"
#include "assert.h"

/* Private typedef -----------------------------------------------------------*/
// ��������Ϣ
typedef struct {
	unsigned char			active;			// �����־
	unsigned char			nouse[3];
	unsigned char			flag;			// must be 0x55
	unsigned char			nouse2[3];
	unsigned int			start;			// start sector
	unsigned int			end;			// end sector
	unsigned char			end0x55;
	unsigned char			end0xAA;
}TBootinfo;


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

/* Private define ------------------------------------------------------------*/			

// BPT�У�BPT_DataType��Ӧ��ֵ
#define RESTYPE_CODE		0x80					// ϵͳ��������
#define RESTYPE_FNT			0x10					// �ֿ���Դ��
#define RESTYPE_PIC			0x11					// ͼƬ��Դ��
#define RESTYPE_FNT_ROM		0x12					// �ֿ���Դ��,��Ҫ��ŵ�MCU FLASH������ֿ�


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

/* Global variables ---------------------------------------------------------*/
unsigned int					dwPictureLBA;
static unsigned char			res_buffer[512];

static FIL						resource_file;		//��Դ�ļ�ר��һ���ļ����
FATFS							fs;
#define res_file				"/resdata.bin"

/* Private function prototypes -----------------------------------------------*/
static int verify_rit(unsigned char *pRit);
//static int verify_data(unsigned int start, TResInfoTable *pResInfoTable);
static int res_loadres(unsigned int dwLBA, unsigned char *pType, unsigned char *pID);
/* Private functions ---------------------------------------------------------*/

/**
* @brief У��Table��CRC32
* @param[in] unsigned char *pRit RIT��һ������
* @return 0:��ȷ
*        -1:����
* @note 
*/
static int verify_rit(unsigned char *pRit)
{
	unsigned int					crc;
	volatile unsigned int			old_crc;

	if( pRit[510] != 0x55 ||pRit[511] != 0xAB)
	{
		return -1;
	}

	old_crc		= ((TResInfoTable*)pRit)->RIT_CheckSum;
	((TResInfoTable*)pRit)->RIT_CheckSum	= 0;

	crc								=  crc32(0, pRit, 512);

	if( crc != old_crc )
	{
		return -1;
	}

	return 0;
}

/**
* @brief У��FAT�е���Դ�Ƿ���ȷ
* @param[in] 
* @return -1:����
*          0:��ȷ
* @note 
*/
#if 0
static int verify_data(unsigned int start, TResInfoTable *pResInfoTable)
{
	unsigned char					checksum;
	unsigned char           		tmpbuf[512];
	int								i,j;

	int								read_adr; 
	//
	// ����binfile��ÿ��������������У���
	//
	checksum					= 0;
	read_adr					= start * 512;                  

	for(i=0; i<pResInfoTable->RIT_CodeSec; i++)
	{
		SD_ReadBlock(read_adr, (unsigned int *)tmpbuf, 512);
		read_adr					+= 512;
		for(j = 0;j<512;j++)
		{
			checksum^=tmpbuf[j];
		}
	}

	if(checksum != pResInfoTable->RIT_DataCRC32)
		return -1;		// data is invalid
	else
		return 0;		// data  is valid
}
#endif


/**
* @brief ��ָ����ַ��ʼ����BPT��������Ƿ���ͬ
*/
static int res_loadres(unsigned int dwLBA, unsigned char *pType, unsigned char *pID)
{
	TResInfoTable			*pResTable;
	
	spi_flash_read(dwLBA,res_buffer,1);
	
	pResTable	= (TResInfoTable*)res_buffer;
	//dwLBA++;

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
*        -1:��ʼ��SPI Flashʧ��
*        -2:SPI Flash���Ҳ�����Դ
*/
int res_init(void)
{
	unsigned int					dwLBA;
	unsigned char					res_type, res_id;
	
	FontList[FNT_ASC_8_16].address	= (unsigned int)fontb_en;
	FontList[FNT_ASC_12_24].address	= (unsigned int)fonta_en;
	FontList[FNT_CHN_16_16].address	= 0;
	FontList[FNT_CHN_24_24].address	= 0;
	dwPictureLBA					= 0;

	
	// װ����Դ
	dwLBA							= RES_START_SECT/512;
	do
	{
		// װ��һ����
		if( res_loadres(dwLBA, &res_type, &res_id) != 0)
		{
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
		// ȡ��һ����Դ��ַ
		dwLBA						= *(unsigned int*)&res_buffer[0x1F4];
	}while(dwLBA != 0xFFFFFFFF);
	
    return 0;
}

/**
* @brief ��FAT�ж�ȡһ����������Դ��������spi_flash_buffer��
* @param[in] unsigned int dwLBAҪ��ȡ����Դ��ַ
* @return unsigned char *���ݴ���ڵ�ַ
*/
unsigned char *read_resdata(unsigned int dwLBA)
{
	if(spi_flash_read(dwLBA, res_buffer, 1) == 0)
		return res_buffer;
	else
		return (unsigned char*)0;
}



/**
* @brief ������Դ��֤
* @note ����������NFCReader�е���������
*/
int res_upgrade(void)
{
	TBootinfo						*pBootInfo;
	TResInfoTable					*pResInfoTable;
	unsigned int   					dwsrcLBA,tempLBA,tempCodeSec,desFlashAddress;	
	unsigned short					res_type;
	unsigned int					i,j;
	unsigned char					eraseFlag = 0;
	unsigned char					tempbuf[512];
        int   rd,ret;

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
	//
	// ��ȡResInfoTable
	//
	dwsrcLBA						= 0;
	desFlashAddress					= RES_START_SECT;
	do
	{
		dwsrcLBA += 1;
		f_lseek(&resource_file,dwsrcLBA*512);

		if( f_read(&resource_file, res_buffer, 512, &rd) != FR_OK )
		{
			f_close(&resource_file);
			return -2;
		}

		pResInfoTable				= (TResInfoTable*)res_buffer;
		
		
		res_type					= pResInfoTable->RIT_DataType;			//��ǰ��Դ������
		tempLBA						= dwsrcLBA;								//��ǰ��Դ��Ϣ��ĵ�ַ
		tempCodeSec					= pResInfoTable->RIT_TotSect;			//��ǰ��Դ�Ĵ�С
		
		// ��һ����Դ��ַ
		dwsrcLBA					= *(unsigned int*)&res_buffer[0x1F4];

		// �ж���Դ����
		//������ֿ���Դ������PIC ��Դ��ô��װ�ص�SPI Flash��ȥ
		if((res_type == RESTYPE_FNT)||(res_type == RESTYPE_PIC))
		{
			//@note��TResInfoTable�������ͬ���ں������Դ����һ��copy����Դ����ȥ
			if(eraseFlag == 0)
			{
				for (i=0;i<RES_FLASH_SIZE/(BLOCK_ERASE_SIZE*512);i++) 
				{
					if (spi_flash_eraseblock(RES_START_SECT+i*BLOCK_ERASE_SIZE*512)) 
					{
						f_close(&resource_file);
						return -1;
					}
				}
				eraseFlag = 1;
			}

			/**
			*  @note ��ResInforTable д�������ʱ��ResInfoTable�е�ָ����һ����Դ��Sector��ַ��Ҫ
			*  ������Դ��SPI Flash�еľ���λ��������
			*/
			
			//�������һ�����⣬�����Դ����֮�����һ��AppCode�����ܵ���Appcode�������Դû�и��½���		
			if(dwsrcLBA != 0xffffffff)
			{
				*(unsigned int*)&res_buffer[0x1F4] += RES_START_SECT/512+1;
			}
			
			// ��TResInfoTable д��SPI Flash�ĵ�һ��Sector
			if( spi_flash_wpage(desFlashAddress,res_buffer) != 0 ||
				spi_flash_wpage(desFlashAddress+256,res_buffer+256) != 0)
			{
				f_close(&resource_file);
				return -1;
			}
			
			spi_flash_read(desFlashAddress/512,tempbuf,1);
			if(memcmp(res_buffer,tempbuf,512)!=0)
			{
				f_close(&resource_file);
				return -1;
			}
			desFlashAddress += 512;
			//��ȡ��Դ���ݲ�ת�Ƶ���Դ��
			for(i=0;i<tempCodeSec;i++)
			{
				/*if( SD_ReadBlock((tempLBA+1+i) * 512, (unsigned int *)spi_flash_buffer, 512) != SD_OK )
				{
					return -1;
				}*/
				f_lseek(&resource_file,(tempLBA+1+i) * 512);

				if( f_read(&resource_file, res_buffer, 512, &rd) != FR_OK )
				{
					f_close(&resource_file);
					return -2;
				}
				
				if( spi_flash_wpage(desFlashAddress,res_buffer) != 0 ||
					spi_flash_wpage(desFlashAddress+256,res_buffer+256) != 0)
				{
					f_close(&resource_file);
					return -1;
				}
				
				spi_flash_read(desFlashAddress/512,tempbuf,1);
				if(memcmp(res_buffer,tempbuf,512)!=0)
				{
					f_close(&resource_file);
					return -1;
				}
				desFlashAddress += 512;
			}					
		}
			
	}while(dwsrcLBA != 0xFFFFFFFF);
	
	f_close(&resource_file);

	f_unlink(res_file);		//���ɾ����Դ�ļ�
	
	return 0; 
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
