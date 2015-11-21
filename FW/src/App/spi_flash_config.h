/**
* @file spi_flash_config.h
* @brief 2.4GPOS��Ŀ��SPI FLASH��ʹ�ù滮���������
*        
* @version V0.0.1
* @author joe.zhou
* @date 2010��4��26��
* @note
*     SPI FLASH��ʹ�ù滮��Ҫ���ݾ���SPI FLASH�Ĵ�С�Լ�Ӧ����ʹ�õ�����Դ��С����
*     ��ʱ֧��MX25L1605D��MX25L3205D������SPI FLASH������
*     
*     ����滮���£�
*	  2M SPI FLASH�� ��SPI FLASH�ĵ�һ��Sector��ʼ���GBK24*24���ֿ���Դ���ݣ����Ŵ��PIC.DAT(�����ͼƬ����)��
*					 ��Լռ��3584Sectors�Ŀռ䣻���Ŵ���ն˲�������Ҫռ��8��Sector�Ŀռ䣻����д�Ž��׼�¼
*                    ����2M��SPI FLASH����̫С������GBK���ֿ�֮�󣬾�û�пռ���GB2312 12*12���ֿ⣬����ֻ�ܰ�
*                    GB2312 12*12���ֿ�ŵ�MCU ��FLASH����
*					 
*	  4M SPI FLASH�� ��SPI FLASH�ĵ�һ��Sector��ʼ���GBK24*24���ֿ���Դ���ݣ����Ŵ��GB2312 12*12�ֿ⣬����
*					 ���PIC.DAT(�����ͼƬ����)�����Ŵ���ն˲�������Ҫռ��8��Sector�Ŀռ䣻����д�Ž��׼�¼
*/

#ifndef _SPI_FLASH_CONFIG_H_
#define _SPI_FLASH_CONFIG_H_

#define MX25L1605D	1		//2MByte
#define MX25L3205D	2		//4MByte

//#define SPI_FLASH_TYPE		MX25L1605D			//Enable this define if use MX25L1605D
#define SPI_FLASH_TYPE		MX25L3205D		//Enable this define if use MX25L3205D

//��Ҫ����SPI Flash �������Ƿ�֧��Sector Erase����֧�־ͱ���ر�����ĺ궨�壬֧�ֿ��Դ򿪣�����Ϊ�洢Record������༷��ռ����
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

//����SPI Flash�Ĳ�����λ
#define		BLOCK_ERASE_SIZE		128				//Sector		64K Block Erase
#define    SECTOR_ERASE_SIZE		8				//Sector		4K Sector Erase

#ifdef SUPPORT_SECTOR_ERASE
//�����С������λ��Sector Erase
#define RECORD_AREA_SIZE		SECTOR_ERASE_SIZE*47	//2 Block Size for Record	         188K				 
#define PARAM_AREA_SIZE			SECTOR_ERASE_SIZE		//1 Sector Size for Terminal Param   4K
#else
//�����С������λ��Block Erase
#define RECORD_AREA_SIZE		BLOCK_ERASE_SIZE*2		//2 Block Size for Record			128K 
#define PARAM_AREA_SIZE			BLOCK_ERASE_SIZE		//1 Block Size for Terminal Param   64K 
#endif


#define RES_START_SECT			0												//Resource start sector
#define PARAM_START_SECT		(RECORD_START_SECT - PARAM_AREA_SIZE)			//Param start sector,	
#define RECORD_START_SECT		(FLASH_SIZE - RECORD_AREA_SIZE)					//Record start sector		the last 256 Sector for Record	
#endif