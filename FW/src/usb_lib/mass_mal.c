/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_app_config.h"
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
#include "mass_mal.h"
//#include "spi_sd.h"
#include "stm32f10x_sdio.h"
#include "spi_flash.h"

#ifdef DUMMY_FAT_FS
#include "dummy_fat16_data.h"
#include <string.h>
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u32 Mass_Memory_Size[2];
u32 Mass_Block_Size[2];
u32 Mass_Block_Count[2];
volatile u32 Status = 0;


//extern SD_CardInfo							SDIO_Info;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_Init(u8 lun)
{
  u16 status = MAL_OK;

  switch (lun)
  {
    case 0:
//      if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_MSD)
//      {
//              Status = SD_Init();
//      Status = SD_GetCardInfo(&SDCardInfo);
//      Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
//      Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
 //     Status = SD_SetDeviceMode(SD_DMA_MODE);

      //SPI_SD_Init();
 //     }

      break;
    default:
      return MAL_FAIL;
  }
  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_Write(u8 lun, u32 Memory_Offset, u32 *Writebuff, u16 Transfer_Length)
{
   unsigned char *pBuf = (unsigned char*)Writebuff;
#ifdef DUMMY_FAT_FS
	int i;
#endif
  switch (lun)
  {
    case 0:
		if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_DUMMY_FAT)
		{
			#ifdef DUMMY_FAT_FS
				if ((Memory_Offset >= DB_FILE_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (DB_FILE_LBA*Mass_Block_Size[lun]+IF_FILE_SIZE)))
				{
					//写接口文件
					//通过如下方法将接收到的命令数据送往命令缓冲区
					for (i = 0; i < Transfer_Length;i++)
					{
						if(PCUsart_InByte(pBuf[i]))
						{
							break;
						}
					}
				}

				break;
			#endif
		}
		else if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_SPI_FLASH)
		{
			if ((Memory_Offset >= FAT_FLASH_SIZE)||((Memory_Offset+Transfer_Length)>FAT_FLASH_SIZE))
			{
				return MAL_FAIL;
			}

			Status = spi_flash_write(fatfs_sector_offset+Memory_Offset/512,pBuf,1);
			if (Status)
			{
				return MAL_FAIL;
			}
			break;
		}
		//else
		//{
		//	Status = SD_WriteBlock(Memory_Offset, Writebuff, Transfer_Length);
		//	if ( Status != SD_OK )
		//	{
		//		return MAL_FAIL;
		//	}      
		//	//SPI_SD_WriteBlock((u8*)Writebuff, Memory_Offset, Transfer_Length);
		//	break;
		//}
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
u16 MAL_Read(u8 lun, u32 Memory_Offset, u32 *Readbuff, u16 Transfer_Length)
{
  switch (lun)
  {
    case 0:
		if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_DUMMY_FAT)
		{
			#ifdef DUMMY_FAT_FS
				if (Memory_Offset < (MBR_LBA*Mass_Block_Size[lun]+512))
				{
					MEMCPY(Readbuff,dummy_MBR+Memory_Offset-MBR_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else if ((Memory_Offset >= DBR_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (DBR_LBA*Mass_Block_Size[lun]+512)))
				{
					MEMCPY(Readbuff,dummy_DBR+Memory_Offset-DBR_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else if ((Memory_Offset >= FAT1_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (FAT1_LBA*Mass_Block_Size[lun]+512)))
				{
					MEMCPY(Readbuff,dummy_FAT+Memory_Offset-FAT1_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else if ((Memory_Offset >= FAT2_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (FAT2_LBA*Mass_Block_Size[lun]+512)))
				{
					MEMCPY(Readbuff,dummy_FAT+Memory_Offset-FAT2_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else if ((Memory_Offset >= FDT_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (FDT_LBA*Mass_Block_Size[lun]+512)))
				{
					MEMCPY(Readbuff,dummy_FDT+Memory_Offset-FDT_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else if ((Memory_Offset >= DB_FILE_LBA*Mass_Block_Size[lun])&&(Memory_Offset < (DB_FILE_LBA*Mass_Block_Size[lun]+IF_FILE_SIZE)))
				{
					MEMCPY(Readbuff,g_send_buff + Memory_Offset-DB_FILE_LBA*Mass_Block_Size[lun],Transfer_Length);
				}
				else
				{
					MEMSET(Readbuff,0,Transfer_Length);
				}
				break;
			#endif
		}
		else if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_SPI_FLASH)
		{
			Status = spi_flash_post_write();
			if (Status)
			{
				return MAL_FAIL;
			}

			if ((Memory_Offset >= FAT_FLASH_SIZE)||((Memory_Offset+Transfer_Length)>FAT_FLASH_SIZE))
			{
				return MAL_FAIL;
			}

			spi_flash_raddr(fatfs_sector_offset*512+Memory_Offset, Transfer_Length,(unsigned char*)Readbuff);
            break;
		}
		//else
		//{
		//	Status = SD_ReadBlock(Memory_Offset, Readbuff, Transfer_Length);
		//	if ( Status != SD_OK )
		//	{
		//		return MAL_FAIL;
		//	}
		//	//SPI_SD_ReadBlock((u8*)Readbuff, Memory_Offset, Transfer_Length);
		//	break;
		//}
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 MAL_GetStatus (u8 lun)
{

#if 1
  //u32 DeviceSizeMul = 0, NumberOfBlocks = 0;

  //u32 temp_block_mul = 0;
//  SD_CSD MSD_csd;
  //u32 DeviceSizeMul = 0;



  if (lun == 0)
  {
	  if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_DUMMY_FAT)
	  {
		#ifdef DUMMY_FAT_FS  
		  Mass_Block_Count[0] = 0x1e500;	//dummy
		  Mass_Block_Size[0] = 512;			//dummy
		  Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
		  return MAL_OK;
		#endif
	  }
	  else if (g_mass_storage_device_type == MASSTORAGE_DEVICE_TYPE_SPI_FLASH)
	  {
			Mass_Block_Count[0] = get_spi_flash_capacity();
			Mass_Block_Size[0] = 512;
			Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
			if (spi_flash_post_write())
			{
				return MAL_FAIL;
			}
			return MAL_OK;
	  }
//	  else
//	  {
//		  if (SD_Init() == SD_OK)
//		  {
//			  SD_GetCardInfo(&SDCardInfo);
//			  SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
//			  DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);
//
//			  if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
//			  {
//				  Mass_Block_Count[0] = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
//			  }
//			  else
//			  {
//				  NumberOfBlocks  = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
//				  Mass_Block_Count[0] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
//			  }
//			  Mass_Block_Size[0]  = 512;
//
//			  Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16)); 
//			  Status = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
//			  if ( Status != SD_OK )
//			  {
//				  return MAL_FAIL;
//			  }
//
//			  Status = SD_SetDeviceMode(SD_DMA_MODE);         
//			  if ( Status != SD_OK )
//			  {
//				  return MAL_FAIL;
//			  }  
//			   return MAL_OK;
//		  }
#if 0
			  SPI_SD_GetCSDRegister(&MSD_csd);
			  DeviceSizeMul = MSD_csd.DeviceSizeMul + 2;
			  temp_block_mul = (1 << MSD_csd.RdBlockLen)/ 512;
			  Mass_Block_Count[0] = ((MSD_csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
			  Mass_Block_Size[0] = 512;
			  //Mass_Memory_Size[0] = (Mass_Block_Count[0] * Mass_Block_Size[0]);

			  Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
			  return MAL_OK;
#endif
//	  }

  }
  return MAL_FAIL;
#endif
}

#endif

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
