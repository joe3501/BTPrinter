/**
* @file spi_flash_config.h
* @brief BT_printer��Ŀ��SPI FLASH��ʹ�ù滮���������
*        
* @version V0.0.1
* @author kent.zhou
* @date 2015��12��01��
* @note
*     
*     ����滮���£�
*	  8M SPI FLASH�� ��SPI FLASH�ĵ�һ��Sector��ʼ��1M�ռ䣬��recmod����ȥ�����������������Ҫ����Ĳ���
*					 ��SPI FLASH�ĵ�1Mƫ�ƿ�ʼ��5M�ռ䣬��spi flash��ײ������ȥ�������������ֿ����ݣ���Ҫ�ӿ��ȡ�ٶȣ���������ײ������ȥ����
*					 ��SPI FLASH�ĵ�6Mƫ�ƿ�ʼ��2M�ռ䣬��FAT�ļ�ϵͳȥ����������մ�U�̽ӿڴ�������������	

*/

#ifndef _SPI_FLASH_CONFIG_H_
#define _SPI_FLASH_CONFIG_H_

#define			BLOCK_ERASE_SIZE		128				//Sector		64K Block Erase

#define			FLASH_SIZE			(8*1024*1024)			//8M

#define PARAM_FLASH_SIZE		((1024-64)*1024)					//1M �������
#define RES_FLASH_SIZE			(5*1024*1024)				//5M �ֿ����Դ�ļ�
#define FAT_FLASH_SIZE			((2*1024+64)*1024)				//2M  FAT�ļ�ϵͳ	

#define PARAM_START_SECT		0									//Param start sector,	
#define RES_START_SECT			PARAM_FLASH_SIZE							//Resource start sector
#define FAT_START_SECT			(RES_START_SECT+RES_FLASH_SIZE)


#endif