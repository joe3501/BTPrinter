
/**
* @file res_spi_new.c
* @brief BT Printer资源管理模块,用于管理字库，图片功能
*        本模块将字库、图片等资源存储于SPI FLASH中。
*
* @version V0.0.1
* @author kent.zhou
* @date 2015年12月01日
* @note
*     每种不同的资源，在一个独立的BPT中，利用BPT_DataType来区分
*
* 
*/
/*
	此模块需要提供的接口是：1.完成将资源从FAT升级到SPI FLASH的功能；
							2.完成资源的安装，也就是说将字库资源和字体形成一种对应关系，图片资源和
							需要用到的图片ID形成一种对应关系
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
// 引导区信息
typedef struct {
	unsigned char			active;			// 激活标志
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
	unsigned short				RIT_ProdID;					// 30 产品ID
	unsigned short				RIT_CodeVersion;    		// 32 引导代码版本
	unsigned short				RIT_Date;            		// 34 更新时间
	unsigned char				RIT_SN[10];            		// 36 序列号
	unsigned int				RIT_TotSect;        		// 40 引导分区占的扇区数
	unsigned int				RIT_CodeSec;        		// 44 引导代码占的扇区数
	unsigned char				RIT_VerifyType;        		// 48 引导代码校验方式
	unsigned char				RIT_Endian;        			// 49 CPU Core
	unsigned short				RIT_DataType;         		// 4A 此份数据的类型（如果不是引导代码，则指引导数据类型）
	unsigned int				RIT_StartAddr;        		// 4C 启动代码在ram中存放的起始地址
	unsigned int				RIT_RunAddr;        		// 50 运行代码时PC跳转的地址
	unsigned char   			RIT_DataID;					// 54 此份资源的ID号
	unsigned char				RIT_Reserved0;				// 55
	unsigned char				RIT_Align;					// 56 此份资源的最小对齐单位，为2^n字节
	unsigned char				RIT_Reserved1;				// 57
	unsigned char				RIT_StrVer[13];				// 58  资源的版本号，格式为20100610_0001
	unsigned char				RIT_Reserved[0x18B];		// 65 
	unsigned int				RIT_DataCRC32;				// 1F0 数据区的CRC32，即从下一个扇区起，到RIT_CodeSec止的所有数据的CRC32值或CRC，此项根据xml文件中的配置来生成。
	unsigned int				RIT_Next;            		// 1F4 下一份RIT在磁盘中的绝对地址
	unsigned int				RIT_CheckSum;        		// 1F8 此份资源表RIT的CRC32校验值。， 
	unsigned int				RIT_TrailSig;       		// 1FC 结束标志(0x00,0x00,0x55,0xAB)
}TResInfoTable;

#pragma pack()

/* Private define ------------------------------------------------------------*/			

// BPT中，BPT_DataType对应的值
#define RESTYPE_CODE		0x80					// 系统升级代码
#define RESTYPE_FNT			0x10					// 字库资源号
#define RESTYPE_PIC			0x11					// 图片资源号
#define RESTYPE_FNT_ROM		0x12					// 字库资源号,需要存放到MCU FLASH区域的字库


#define MAX_FNT_CNT				FNT_CHN_24_24

/**
*	@brief  刷卡机项目中用到的字体信息表
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

static FIL						resource_file;		//资源文件专享一个文件句柄
FATFS							fs;
#define res_file				"/resdata.bin"

/* Private function prototypes -----------------------------------------------*/
static int verify_rit(unsigned char *pRit);
//static int verify_data(unsigned int start, TResInfoTable *pResInfoTable);
static int res_loadres(unsigned int dwLBA, unsigned char *pType, unsigned char *pID);
/* Private functions ---------------------------------------------------------*/

/**
* @brief 校验Table的CRC32
* @param[in] unsigned char *pRit RIT表，一个扇区
* @return 0:正确
*        -1:错误
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
* @brief 校验FAT中的资源是否正确
* @param[in] 
* @return -1:错误
*          0:正确
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
	// 读入binfile的每个扇区，并计算校验和
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
* @brief 从指定地址开始载入BPT，并检查是否相同
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
* @brief 安装中文字库
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
* @brief 找到了图片资源，根据ID，得到地址
*/
void setup_picture(unsigned int dwLBA)
{
	dwPictureLBA					= dwLBA;
}
/**
* @brief 资源模块初始化
* @return 0:成功初始化
*        -1:初始化SPI Flash失败
*        -2:SPI Flash中找不到资源
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

	
	// 装入资源
	dwLBA							= RES_START_SECT/512;
	do
	{
		// 装入一个表
		if( res_loadres(dwLBA, &res_type, &res_id) != 0)
		{
			return -2;
		}
		
		// 判断资源类型
		if(RESTYPE_FNT == res_type)
		{
			setup_font(dwLBA+1, res_id);		// 安装字体
		}
		else if(RESTYPE_PIC == res_type)
		{
			setup_picture(dwLBA + 1);
		}	
		// 取下一份资源地址
		dwLBA						= *(unsigned int*)&res_buffer[0x1F4];
	}while(dwLBA != 0xFFFFFFFF);
	
    return 0;
}

/**
* @brief 从FAT中读取一个扇区的资源，并放在spi_flash_buffer中
* @param[in] unsigned int dwLBA要读取的资源地址
* @return unsigned char *数据存放在地址
*/
unsigned char *read_resdata(unsigned int dwLBA)
{
	if(spi_flash_read(dwLBA, res_buffer, 1) == 0)
		return res_buffer;
	else
		return (unsigned char*)0;
}



/**
* @brief 进行资源验证
* @note 本代码沿用NFCReader中的升级程序
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

	//挂载文件系统
	f_mount(0, &fs);										// 装载文件系统

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
	// 1. 校验资源文件
	if( f_read(&resource_file, res_buffer, 512, &rd) != FR_OK )
	{
		f_close(&resource_file);
		return -2;
	}


	// 校验文件头
	if( res_buffer[0] != 'J' || res_buffer[1] != 'B' || res_buffer[2] != 'L' || res_buffer[3] != '3' )
	{
		f_close(&resource_file);
		return -2;
	}
	//
	// 读取ResInfoTable
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
		
		
		res_type					= pResInfoTable->RIT_DataType;			//当前资源的类型
		tempLBA						= dwsrcLBA;								//当前资源信息表的地址
		tempCodeSec					= pResInfoTable->RIT_TotSect;			//当前资源的大小
		
		// 下一份资源地址
		dwsrcLBA					= *(unsigned int*)&res_buffer[0x1F4];

		// 判断资源类型
		//如果是字库资源或者是PIC 资源那么就装载到SPI Flash中去
		if((res_type == RESTYPE_FNT)||(res_type == RESTYPE_PIC))
		{
			//@note将TResInfoTable这个表连同跟在后面的资源数据一起copy到资源区域去
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
			*  @note 将ResInforTable 写入参数区时，ResInfoTable中的指向下一个资源的Sector地址需要
			*  根据资源在SPI Flash中的具体位置作调整
			*/
			
			//这里存在一个问题，如果资源链表之间夹了一个AppCode，可能导致Appcode后面的资源没有更新进来		
			if(dwsrcLBA != 0xffffffff)
			{
				*(unsigned int*)&res_buffer[0x1F4] += RES_START_SECT/512+1;
			}
			
			// 将TResInfoTable 写入SPI Flash的第一个Sector
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
			//读取资源数据并转移到资源区
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

	f_unlink(res_file);		//最后删除资源文件
	
	return 0; 
}	


/**
* @brief 从资源文件中获取一个字符的字模数据
* @param[in] unsigned char font_type		字体类型	
* @param[in] unsigned char *c				字符编码	
* @param[in] unsigned char *pBuf			返回字模数据的缓冲区	
* @param[in] unsigned int size				字模数据的大小	
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
		GBK码对字库中偏移量的计算公式为：

		GBKindex = ((unsigned char)GBKword[0]-129)*190 +

		((unsigned char)GBKword[1]-64) - (unsigned char)GBKword[1]/128;

		*/
		//offset				= ((c[0] - 0xA1) & 0x7F) * 94 + ((c[1]-0xA1)&0x7F);		//GB2312字符集偏移计算公式

		offset				= (c[0] - 129) * 190 + (c[1]-64) - (c[1]/128);				//GBK字符集偏移计算公式
		pInfo				= &FontList[font_type];


		if(pInfo->address == 0)
			return;

		offset				*= pInfo->size;
		pData				= read_resdata(pInfo->address + (offset/512));	// 计算所在的扇区
		pData				+= offset & 0x1FF;								// 计算扇区内的偏移

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
