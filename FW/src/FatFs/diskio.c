/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "stm32f10x_lib.h"
//#include "sdcard.h"
#include "spi_flash.h"
/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

//#define ATA		0
//#define MMC		1
//#define USB		2

#define MMC			1	//利用MMC/SD卡实现的文件系统
#define SPI_FLASH	0	//利用SPI_FLASH实现的文件系统

//static int							bus_in_sdio = 0;				// 1:用SDIO连接SD卡 0:用SPI连接SD卡
//SD_CardInfo							SDIO_Info;
//unsigned int						SDIO_Block_Count;
//int									SDIO_Block_Size;
//int									SDIO_Memory_Size;
//unsigned char						sdbuffer[512];

//static int check_sdcard(void);
DSTATUS disk_initialize ( BYTE drv );
DSTATUS disk_unmount(BYTE drv);
DWORD get_fattime (void);
DSTATUS disk_status ( BYTE drv );
DRESULT disk_read ( BYTE drv, BYTE *buff, DWORD sector, BYTE count );
DRESULT disk_write ( BYTE drv, const BYTE *buff, DWORD sector, BYTE count );
DRESULT disk_ioctl ( BYTE drv, BYTE ctrl, void *buff);

/**
* @brief 检查sd卡是否就绪
* @return 0:检测成功
*        -1:检测失败
*/
#if 0
static int check_sdcard(void)
{
	unsigned int				DeviceSizeMul = 0, NumberOfBlocks = 0;
	SD_Error					Status;

	if (SD_Init() == SD_OK)
	{
		SD_GetCardInfo(&SDIO_Info);
		SD_SelectDeselect((unsigned int) (SDIO_Info.RCA << 16));
		DeviceSizeMul			= (SDIO_Info.SD_csd.DeviceSizeMul + 2);

		if(SDIO_Info.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
		{
			SDIO_Block_Count = (SDIO_Info.SD_csd.DeviceSize + 1) * 1024;
			SDIO_Block_Size			= 1;
		}
		else
		{
			NumberOfBlocks		= ((1 << (SDIO_Info.SD_csd.RdBlockLen)) / 512);
			SDIO_Block_Count	= ((SDIO_Info.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
			SDIO_Block_Size			= 512;
		}


		Status					= SD_SelectDeselect((unsigned int) (SDIO_Info.RCA << 16)); 
		Status					= SD_EnableWideBusOperation(SDIO_BusWide_4b); 
		if ( Status != SD_OK )
		{
			return -1;
		}

		Status					= SD_SetDeviceMode(SD_DMA_MODE);         
		if ( Status != SD_OK )
		{
			return -1;
		}
		SDIO_Memory_Size		= SDIO_Block_Count * SDIO_Block_Size;
		if( SDIO_Memory_Size >= (2*1024 * 1024 * 1024)) // * 512))
		{
			return 0;			// 卡大于2G
		}
		else
			return -1;
	}
	return -1;
}
#endif
/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	int ret;
	//switch(drv)
	//{
	//case MMC:
	//	bus_in_sdio	= 1;
	//	ret = check_sdcard();
	//	break;
	//case SPI_FLASH:
		ret = spi_flash_init();
	//}
	
	if( ret == 0 )
		return RES_OK;
	return RES_ERROR;
}

DSTATUS disk_unmount(BYTE drv)
{
	//switch(drv)
	//{
	//case MMC:
	//	bus_in_sdio						= 0;
	//	break;
	//case SPI_FLASH:
		if (spi_flash_post_write())
		{
			return  RES_ERROR;
		}
		return RES_OK;
	//}

	//return RES_OK;
}


DWORD get_fattime (void)
{
	return 0;
}
/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
//	DSTATUS stat;
	int sta;
	//switch(drv)
	//{
	//case MMC:
	//	if(bus_in_sdio == 0)
	//	{
	//		return disk_initialize(drv);
	//	}

	//	if( SD_SendStatus(&sta) == SD_OK )
	//	{
	//		return RES_OK;
	//	}
	//	break;

	//case SPI_FLASH:
		if (spi_flash_post_write())
		{
			return  RES_ERROR;
		}
		return RES_OK;
	//}
	

	//return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
	DRESULT							res = 0;
//	int		i;
//	int result;

//	switch (drv) {
//	case ATA :
//		result = ATA_disk_read(buff, sector, count);
		// translate the reslut code here

//		return res;

//	case MMC :
//		return MMC_disk_read(buff, sector, count);
		// translate the reslut code here

	//switch (drv)
	//{
	//case MMC:
	//	if(count == 1)
	//	{
	//		res = SD_ReadBlock(sector * 512, (unsigned int*)buff, 512);
	//	}
	//	else if(count > 1)
	//	{
	//		res = SD_ReadMultiBlocks(sector * 512, (unsigned int*)buff, 512, count);
	//	}
	//	if( res != SD_OK ) return RES_ERROR;
        //        return RES_OK;

	//case SPI_FLASH:
		//if (spi_flash_post_write())
		//{
  //            return RES_ERROR;
		//}
		//res = spi_flash_rpage(sector, count, (unsigned char*)buff);
	if ((sector >= FAT_FLASH_SIZE/512) || ((sector+count) > FAT_FLASH_SIZE/512))
	{
		return RES_ERROR; 
	}

	res  = spi_flash_read(fatfs_sector_offset+sector,(unsigned char*)buff,count);
	if (res)
	{
		return RES_ERROR;
	}
		
		//break;
	//}

	if (res)
	{
		return RES_ERROR;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
	DRESULT							res;
//	int result;
//	int i;

//	switch (drv) {
//	case ATA :
//		result = ATA_disk_write(buff, sector, count);
		// translate the reslut code here

//		return res;

//	case MMC :
//		return MMC_disk_write(buff, sector, count);
//	return SD_WriteBlock(sector * 512, buff, )

	//if (MMC == drv)
	//{
	//	if ( 1==count)
	//	{
	//		res = SD_WriteBlock(	sector* 512, (unsigned int*)buff, 512	);
	//	} 
	//	else if( count>1)
	//	{
	//		res = SD_WriteMultiBlocks(	sector * 512,(unsigned int*)buff,512,count	);
	//	}

	//	if( res == SD_OK)
	//		return RES_OK;
	//	else
	//		return RES_ERROR;
	//}
	//else if(SPI_FLASH == drv)
	{
		if ((sector >= FAT_FLASH_SIZE/512) || ((sector+count) > FAT_FLASH_SIZE/512))
		{
			return RES_ERROR; 
		}
		res  = spi_flash_write(fatfs_sector_offset+sector,(unsigned char*)buff,count);
		if (res)
		{
			return RES_ERROR;
		}
		return RES_OK;
	}

}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
// 返回磁盘状态
// 
DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
#if 0
//	DRESULT res;
//	int result;

//	switch (drv) {
//	case ATA :
		// pre-process here

//		result = ATA_disk_ioctl(ctrl, buff);
		// post-process here

//		return res;

//	case MMC :
		// pre-process here

//		return MMC_disk_ioctl(ctrl, buff);
		// post-process here

//		return res;

//	case USB :
		// pre-process here

//		result = USB_disk_ioctl(ctrl, buff);
		// post-process here

//		return res;
//	}
//	return RES_PARERR;
	return RES_OK;
#endif
	DRESULT							res = RES_ERROR;
	//SD_CardInfo						SDCardInfo;

	//if (drv>=_DRIVES)
	//{
	//	return RES_PARERR; 
	//}
	//FATFS目前版本仅需处理CTRL_SYNC，GET_SECTOR_COUNT，GET_BLOCK_SIZ三个命令
	switch(ctrl)
	{
	case CTRL_SYNC:
		//if (MMC == drv)
		//{
		//	if(SD_GetTransferState()==SD_NO_TRANSFER)
		//	{
		//		res = RES_OK;
		//	}
		//	else
		//	{
		//		res = RES_ERROR;
		//	}
		//}
		//else if (SPI_FLASH == drv)
		{
			/*if (spi_flash_wait_for_write_end())
			{
				res = RES_ERROR;
			}
			res = RES_OK;*/

			if (spi_flash_post_write())
			{
				res = RES_ERROR;
			}
			res = RES_OK;
		}
		break;

	case GET_BLOCK_SIZE:
		//if (MMC == drv)
		//{
		//	*(WORD*)buff = 1;
		//	res = RES_OK;
		//}
		//else if (SPI_FLASH == drv)
		{
			*(WORD*)buff = 1;		
			res = RES_OK;
		}
		break;

	case GET_SECTOR_COUNT: //读卡容量
		//if(MMC == drv)
		//{
		//	if (SD_GetCardInfo(&SDCardInfo)==SD_OK)//读sd卡信息
		//	{
		//		*(DWORD*)buff =SDCardInfo.CardCapacity /SDCardInfo.CardBlockSize;
		//		res = RES_OK;
		//	}
		//	else
		//	{
		//		res = RES_ERROR ;
		//	}
		//}
		//else if (SPI_FLASH == drv)
		{
			*(DWORD*)buff = get_spi_flash_capacity();
			res = RES_OK;
		}
		break;

	default:
		res = RES_PARERR;
		break;
	}
	return res;
}

