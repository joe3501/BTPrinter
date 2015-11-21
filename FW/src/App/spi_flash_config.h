/**
* @file spi_flash_config.h
* @brief 2.4GPOS项目中SPI FLASH的使用规划及相关配置
*        
* @version V0.0.1
* @author joe.zhou
* @date 2010年4月26日
* @note
*     SPI FLASH的使用规划需要根据具体SPI FLASH的大小以及应用中使用到的资源大小来定
*     暂时支持MX25L1605D和MX25L3205D这两款SPI FLASH的配置
*     
*     具体规划如下：
*	  2M SPI FLASH： 从SPI FLASH的第一个Sector开始存放GBK24*24的字库资源数据，接着存放PIC.DAT(打包的图片数据)，
*					 大约占用3584Sectors的空间；接着存放终端参数，需要占用8个Sector的空间；最后有存放交易记录
*                    由于2M的SPI FLASH容量太小，放了GBK的字库之后，就没有空间存放GB2312 12*12的字库，所以只能把
*                    GB2312 12*12的字库放到MCU 的FLASH区域。
*					 
*	  4M SPI FLASH： 从SPI FLASH的第一个Sector开始存放GBK24*24的字库资源数据，接着存放GB2312 12*12字库，接着
*					 存放PIC.DAT(打包的图片数据)，接着存放终端参数，需要占用8个Sector的空间；最后有存放交易记录
*/

#ifndef _SPI_FLASH_CONFIG_H_
#define _SPI_FLASH_CONFIG_H_

#define MX25L1605D	1		//2MByte
#define MX25L3205D	2		//4MByte

//#define SPI_FLASH_TYPE		MX25L1605D			//Enable this define if use MX25L1605D
#define SPI_FLASH_TYPE		MX25L3205D		//Enable this define if use MX25L3205D

//需要依据SPI Flash 的驱动是否支持Sector Erase，不支持就必须关闭下面的宏定义，支持可以打开，可以为存储Record的区域多挤点空间出来
//#define SUPPORT_SECTOR_ERASE					//if spi_flash.c support Sector Erase(SE),this define can be enabled
												//if spi_flash.c not support Sector Erase(SE),this define must be disabled
#define	SECTOR_SIZE			512					//BYTE

#if(SPI_FLASH_TYPE == MX25L1605D)
	#define			FLASH_SIZE		4096			//Sector
#elif(SPI_FLASH_TYPE == MX25L3205D)
	#define			FLASH_SIZE		8192			//Sector
#else
	#define			FLASH_SIZE		0				//Sector
#endif

//定义SPI Flash的擦除单位
#define		BLOCK_ERASE_SIZE		128				//Sector		64K Block Erase
#define    SECTOR_ERASE_SIZE		8				//Sector		4K Sector Erase

#ifdef SUPPORT_SECTOR_ERASE
//如果最小擦除单位是Sector Erase
#define RECORD_AREA_SIZE		SECTOR_ERASE_SIZE*47	//2 Block Size for Record	         188K				 
#define PARAM_AREA_SIZE			SECTOR_ERASE_SIZE		//1 Sector Size for Terminal Param   4K
#else
//如果最小擦除单位是Block Erase
#define RECORD_AREA_SIZE		BLOCK_ERASE_SIZE*2		//2 Block Size for Record			128K 
#define PARAM_AREA_SIZE			BLOCK_ERASE_SIZE		//1 Block Size for Terminal Param   64K 
#endif


#define RES_START_SECT			0												//Resource start sector
#define PARAM_START_SECT		(RECORD_START_SECT - PARAM_AREA_SIZE)			//Param start sector,	
#define RECORD_START_SECT		(FLASH_SIZE - RECORD_AREA_SIZE)					//Record start sector		the last 256 Sector for Record	
#endif