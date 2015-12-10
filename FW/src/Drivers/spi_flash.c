/**
***************************************************************************
* @file  spi_flash.c
* @brief SPI Flash 底层驱动
***************************************************************************
*
* @version V0.0.1
* @author kent.zhou
* @date 2010年03月28日
* @note 
*
***************************************************************************
*
* @copy
*
* 此代码为深圳合杰电子有限公司项目代码，任何人及公司未经许可不得
* 复制传播，或用于本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2010 heroje</center></h1>
*/

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct tagSPIFlashID
{
	unsigned char				Manufacturer;
	unsigned char				MemoryType;
	unsigned char				Capacity;
	unsigned int				FlashSize;	
} TSPIFlashID;

/* Private define ------------------------------------------------------------*/

#define WRITE      0x02  /* Write to Memory instruction, Page Program*/
#define WRSR       0x01  /* Write Status Register instruction */
#define WREN       0x06  /* Write enable instruction */

#define READ       0x03  /* Read from Memory instruction */
#define RDSR       0x05  /* Read Status Register instruction  */
#define RDID       0x9F  /* Read identification */
#define SE         0x20  /* Sector Erase instruction 4K*/
#define BE		   0xD8  /* Block Erase instruction 64K */
#define CE         0xC7  /* Chip Erase instruction */

#define WIP_Flag   0x01  /* Write In Progress (WIP) flag */

#define Dummy_Byte 0xA5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const TSPIFlashID id_array[] = {
	{0xEF, 0x40, 0x17, 0x800000},			// KH25L1605DM2C
	{0xEF, 0x30, 0x16, 0x200000},			// KH25L1605DM2C
	{0xC8, 0x30, 0x13, 0x080000},			// GigaDevice GD25D40	《GD25DXX_Rev1.1.pdf》
	{0xC8, 0x30, 0x14, 0x100000},			// GigaDevice GD25D80	《GD25DXX_Rev1.1.pdf》
	{0xC8, 0x40, 0x15, 0x200000},			// GigaDevice GD25Q16   《GD25Q16_Rev1.1.pdf》
	{0xC2, 0x20, 0x15, 0x200000},			// MX25L1605D 《MX25L1605D-3205D-6405D-1.4》
	{0xC2, 0x20, 0x16, 0x400000},			// MX25L3205D 《MX25L1605D-3205D-6405D-1.4》
	{0xC2, 0x20, 0x17, 0x800000},			// MX25L6405D 《MX25L1605D-3205D-6405D-1.4》 128Block/Chip,16Sector/Block,4KByte/sector
	//{0x01, 0x02, 0x12},			// S25FL040A uniform《s25fl040a_00_b3.pdf》
	//{0x01, 0x02, 0x25},			// S25FL040A TopBoot《s25fl040a_00_b3.pdf》
	//{0x01, 0x02, 0x26},			// S25FL040A BottomBoot《s25fl040a_00_b3.pdf》
	//{0x00, 0x00, 0x00},
};
static unsigned char	flash_state	= 0;
unsigned int			flasize;

static unsigned char buffer[4096];		//4K缓存
static int	 current_cache_block;		//当前被缓存的Block
unsigned int			recmod_flasize;
unsigned int			fatfs_sector_offset;

/* Private function prototypes -----------------------------------------------*/
int		spi_flash_page_write(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite);
/*----- Low layer function -----*/
unsigned char spi_flash_read_byte(void);
unsigned char spi_flash_send_byte(unsigned char byte);
unsigned short spi_flash_send_halfword(unsigned short HalfWord);
void spi_flash_write_enable(void);
int spi_flash_wait_for_write_end(void);

/* Private functions ---------------------------------------------------------*/
/*------------------ High layer function ----------------*/

/**
* @brief  Initializes the peripherals used by the SPI FLASH driver.
* @param  None
* @retval : 0 成功
*          -1 失败
*/
int spi_flash_init(void)
{
	SPI_InitTypeDef		SPI_InitStructure;
	GPIO_InitTypeDef	GPIO_InitStructure;
	TSPIFlashID			spi_flash_id;
	const TSPIFlashID	*pSPIFLASHID;

	if (flash_state == 1)
	{
		return 0;
	}
	/* Enable SPI2 and GPIO clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CS, ENABLE);

	/* Configure SPI2 pins: SCK, MISO and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure I/O for Flash Chip select */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_CS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_CS, &GPIO_InitStructure);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* SPI2 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	/* Enable SPI2  */
	SPI_Cmd(SPI2, ENABLE);

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "RDID " instruction */
	spi_flash_send_byte(RDID);

	/* Read a byte from the FLASH */
	spi_flash_id.Manufacturer = spi_flash_send_byte(Dummy_Byte);

	/* Read a byte from the FLASH */
	spi_flash_id.MemoryType = spi_flash_send_byte(Dummy_Byte);

	/* Read a byte from the FLASH */
	spi_flash_id.Capacity = spi_flash_send_byte(Dummy_Byte);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	pSPIFLASHID	= &id_array[0];

	while(pSPIFLASHID->Manufacturer != 0x00)
	{
		if( pSPIFLASHID->Manufacturer == spi_flash_id.Manufacturer &&
			pSPIFLASHID->MemoryType == spi_flash_id.MemoryType &&
			pSPIFLASHID->Capacity == spi_flash_id.Capacity )
		{
			flasize	= pSPIFLASHID->FlashSize;
			if (flasize == 8*1024*1024)
			{
				//flasize -= 1024*1024;
				recmod_flasize = PARAM_FLASH_SIZE;
				fatfs_sector_offset = FAT_START_SECT/512;
			}
			else
			{
				return -1;
			}
			break;
		}
		pSPIFLASHID++;
	}

	if(pSPIFLASHID->Manufacturer == 0x00)
	{
		flash_state	= 0;
		return -1;									// 失败
	}

	flash_state	= 1;
	current_cache_block = -1;
	//spi_flash_busy_flag = 0;
	return 0;										// 成功
}

/**
* @brief  CLose the peripherals used by the SPI FLASH driver.
* @param  None
* @retval : None
*/
void spi_flash_close(void)
{
	/* Disable SPI2  */
	SPI_Cmd(SPI2, ENABLE);

	/* Disable SPI2 clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_CS, DISABLE);
}

/**
* @brief  Check the spi flash state
* @param  None
* @retval	0: Flash已经正常初始化
*		    -1: Flash尚未初始化
*/
int spi_flash_valid(void)
{
	if (flash_state == 0)
	{
		if (spi_flash_init())
		{
			return -1;
		}

		//spi_flash_erase();		//for debug

		flash_state = 1;
	}
	return 0;
}


/**
* @brief  Erases the specified FLASH sector 4K.
* @param SectorAddr: address of the sector to erase.
* @retval	0: 成功
*		   -1: 超时
*/
int spi_flash_erasesector(unsigned int SectorAddr)
{
	/* Send write enable instruction */
	spi_flash_write_enable();

	/* Sector Erase */
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Sector Erase instruction */
	spi_flash_send_byte(SE);
	/* Send SectorAddr high nibble address byte */
	spi_flash_send_byte((SectorAddr & 0xFF0000) >> 16);
	/* Send SectorAddr medium nibble address byte */
	spi_flash_send_byte((SectorAddr & 0xFF00) >> 8);
	/* Send SectorAddr low nibble address byte */
	spi_flash_send_byte(SectorAddr & 0xFF);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	return spi_flash_wait_for_write_end();
}


/**
* @brief  Erases the specified FLASH block 64K.
* @param SectorAddr: address of the block to erase.
* @retval	0: 成功
*		   -1: 超时
*/
int spi_flash_eraseblock(unsigned int BlockAddr)
{
	/* Send write enable instruction */
	spi_flash_write_enable();

	/* Sector Erase */
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Sector Erase instruction */
	spi_flash_send_byte(BE);
	/* Send SectorAddr high nibble address byte */
	spi_flash_send_byte((BlockAddr & 0xFF0000) >> 16);
	/* Send SectorAddr medium nibble address byte */
	spi_flash_send_byte((BlockAddr & 0xFF00) >> 8);
	/* Send SectorAddr low nibble address byte */
	spi_flash_send_byte(BlockAddr & 0xFF);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	return spi_flash_wait_for_write_end();
}

/**
* @brief  Erases the entire FLASH.
* @param  None
* @retval 0: 成功
*        -1: 超时
*/
int spi_flash_erase(void)
{
	/* Send write enable instruction */
	spi_flash_write_enable();

	/* Bulk Erase */
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send Bulk Erase instruction  */
	spi_flash_send_byte(CE);
	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	return spi_flash_wait_for_write_end();
}


/**
* @brief 读spi flash指定扇区数据
* @param [in] lba    : 以扇区为单位的地址
* @param [in] secnum : 需要读取的扇区个数
* @param [out] buf   : 读取数据存入的缓冲区
* @retval  0: 成功
*/
int	spi_flash_rpage(int lba, int secnum, unsigned char *buf)
{
	int		i;
        
    if(secnum == 0) return 0;
          
	lba		*= 0x200;
	//while(spi_flash_busy_flag);
	//spi_flash_busy_flag = 1;

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read from Memory " instruction */
	spi_flash_send_byte(READ);

	/* Send ReadAddr high nibble address byte to read from */
	spi_flash_send_byte((lba & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte to read from */
	spi_flash_send_byte((lba & 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte to read from */
	spi_flash_send_byte(lba & 0xFF);

	MEMSET((void *)buf, 0 , 512);
	for(i=0; i<secnum * 512; i++)
	{
		*buf	= spi_flash_send_byte(Dummy_Byte);
		buf++;
	}

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	return 0;	
}


/**
* @brief  Writes one page byte to the FLASH with a single WRITE
*         cycle(Page WRITE sequence). The number of byte can't exceed
*         the FLASH page size.
* @param pBuffer : pointer to the buffer  containing the data to be
*                  written to the FLASH.
* @param WriteAddr : FLASH's internal address to write to.
* @retval  0: 成功
*         -1: 超时
*/
int spi_flash_wpage(unsigned int WriteAddr, unsigned char *pBuffer)
{
	int		i;

	WriteAddr	&= SPI_FLASH_PageMsk;
	/* Enable the write access to the FLASH */
	spi_flash_write_enable();

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send "Write to Memory " instruction */
	spi_flash_send_byte(WRITE);
	/* Send WriteAddr high nibble address byte to write to */
	spi_flash_send_byte((WriteAddr & 0xFF0000) >> 16);
	/* Send WriteAddr medium nibble address byte to write to */
	spi_flash_send_byte((WriteAddr & 0xFF00) >> 8);
	/* Send WriteAddr low nibble address byte to write to */
	spi_flash_send_byte(WriteAddr & 0xFF);

	/* while there is data to be written on the FLASH */
	for ( i = 0; i < SPI_FLASH_PageSize; i++)
	{
		/* Send the current byte */
		spi_flash_send_byte(*pBuffer);
		/* Point on the next byte to be written */
		pBuffer++;
	}

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	return spi_flash_wait_for_write_end();
}


/**
* @brief  Writes more than one byte to the FLASH with a single WRITE
*         cycle(Page WRITE sequence). The number of byte can't exceed
*         the FLASH page size.
* @param pBuffer : pointer to the buffer  containing the data to be
*                  written to the FLASH.
* @param WriteAddr : FLASH's internal address to write to.
* @param NumByteToWrite : number of bytes to write to the FLASH,
*                       must be equal or less than "SPI_FLASH_PageSize" value.
* @retval  0: 成功
*         -1: 超时
*/
int spi_flash_page_write(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short NumByteToWrite)
{
	/* Enable the write access to the FLASH */
	spi_flash_write_enable();

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();
	/* Send "Write to Memory " instruction */
	spi_flash_send_byte(WRITE);
	/* Send WriteAddr high nibble address byte to write to */
	spi_flash_send_byte((WriteAddr & 0xFF0000) >> 16);
	/* Send WriteAddr medium nibble address byte to write to */
	spi_flash_send_byte((WriteAddr & 0xFF00) >> 8);
	/* Send WriteAddr low nibble address byte to write to */
	spi_flash_send_byte(WriteAddr & 0xFF);

	/* while there is data to be written on the FLASH */
	while (NumByteToWrite--)
	{
		/* Send the current byte */
		spi_flash_send_byte(*pBuffer);
		/* Point on the next byte to be written */
		pBuffer++;
	}

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	/* Wait the end of Flash writing */
	return spi_flash_wait_for_write_end();
}

/**
* @brief  Writes block of data to the FLASH. In this function, the
*         number of WRITE cycles are reduced, using Page WRITE sequence.
* @param pBuffer : pointer to the buffer  containing the data to be
*                  written to the FLASH.
* @param WriteAddr : FLASH's internal address to write to.
* @param NumByteToWrite : number of bytes to write to the FLASH.
* @retval  0: 成功
*         -1: 失败
*/
int spi_flash_waddr(unsigned int WriteAddr, unsigned int NumByteToWrite, unsigned char *pBuffer)
{
	unsigned char NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

	Addr = WriteAddr % SPI_FLASH_PageSize;
	count = SPI_FLASH_PageSize - Addr;
	NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
	NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

	if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			if (-1 == spi_flash_page_write(pBuffer, WriteAddr, NumByteToWrite))
			{
				return -1;
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			while (NumOfPage--)
			{
				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, SPI_FLASH_PageSize))
				{
					return -1;
				}				
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			if (-1 == spi_flash_page_write(pBuffer, WriteAddr, NumOfSingle))
			{
				return -1;
			}			
		}
	}
	else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
	{
		if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
		{
			if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
			{
				temp = NumOfSingle - count;

				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, count))
				{
					return -1;
				}				
				WriteAddr +=  count;
				pBuffer += count;

				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, temp))
				{
					return -1;
				}				
			}
			else
			{
				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, NumByteToWrite))
				{
					return -1;
				}
			}
		}
		else /* NumByteToWrite > SPI_FLASH_PageSize */
		{
			NumByteToWrite -= count;
			NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
			NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

			if (-1 == spi_flash_page_write(pBuffer, WriteAddr, count))
			{
				return -1;
			}
			
			WriteAddr +=  count;
			pBuffer += count;

			while (NumOfPage--)
			{
				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, SPI_FLASH_PageSize))
				{
					return -1;
				}
			
				WriteAddr +=  SPI_FLASH_PageSize;
				pBuffer += SPI_FLASH_PageSize;
			}
			if (NumOfSingle != 0)
			{
				if (-1 == spi_flash_page_write(pBuffer, WriteAddr, NumOfSingle))
				{
					return -1;
				}				
			}
		}
	}
	return 0;
}

/**
* @brief  Reads a block of data from the FLASH.
* @param pBuffer : pointer to the buffer that receives the data read
*                  from the FLASH.
* @param ReadAddr : FLASH's internal address to read from.
* @param NumByteToRead : number of bytes to read from the FLASH.
* @retval : None
*/
void spi_flash_raddr(unsigned int ReadAddr,unsigned int NumByteToRead, unsigned char *pBuffer)
{
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read from Memory " instruction */
	spi_flash_send_byte(READ);

	/* Send ReadAddr high nibble address byte to read from */
	spi_flash_send_byte((ReadAddr & 0xFF0000) >> 16);
	/* Send ReadAddr medium nibble address byte to read from */
	spi_flash_send_byte((ReadAddr& 0xFF00) >> 8);
	/* Send ReadAddr low nibble address byte to read from */
	spi_flash_send_byte(ReadAddr & 0xFF);

	while (NumByteToRead--) /* while there is data to be read */
	{
		/* Read a byte from the FLASH */
		*pBuffer = spi_flash_send_byte(Dummy_Byte);
		/* Point to the next location where the byte read will be saved */
		pBuffer++;
	}

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

/**
* @brief  Reads FLASH identification.
* @param  None
* @retval : FLASH identification
*/
unsigned int spi_flash_read_id(void)
{
	unsigned int Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;

	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "RDID " instruction */
	spi_flash_send_byte(0x9F);

	/* Read a byte from the FLASH */
	Temp0 = spi_flash_send_byte(Dummy_Byte);

	/* Read a byte from the FLASH */
	Temp1 = spi_flash_send_byte(Dummy_Byte);

	/* Read a byte from the FLASH */
	Temp2 = spi_flash_send_byte(Dummy_Byte);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();

	Temp = (Temp0 << 16) | (Temp1 << 8) | Temp2;

	return Temp;
}

/*------------------ Low layer function ----------------*/
/**
* @brief  Reads a byte from the SPI Flash.
*   This function must be used only if the Start_Read_Sequence
*   function has been previously called.
* @param  None
* @retval : Byte Read from the SPI Flash.
*/
unsigned char spi_flash_read_byte(void)
{
	return (spi_flash_send_byte(Dummy_Byte));
}

/**
* @brief  Sends a byte through the SPI interface and return the byte
*   received from the SPI bus.
* @param byte : byte to send.
* @retval : The value of the received byte.
*/
unsigned char spi_flash_send_byte(unsigned char byte)
{
         volatile short			i = 0;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Send byte through the SPI2 peripheral */
	SPI_I2S_SendData(SPI2, byte);

	/* Wait to receive a byte */
	//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
        //for(i=0; i<10; i++);
	for(i=0; i<5; i++);

	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI2);
}

/**
* @brief  Sends a Half Word through the SPI interface and return the
*         Half Word received from the SPI bus.
* @param HalfWord : Half Word to send.
* @retval : The value of the received Half Word.
*/
unsigned short spi_flash_send_halfword(unsigned short HalfWord)
{
        volatile short			i = 0;
	/* Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);

	/* Send Half Word through the SPI2 peripheral */
	SPI_I2S_SendData(SPI2, HalfWord);

	/* Wait to receive a Half Word */
	//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
        for(i=0; i<10; i++);
        
	/* Return the Half Word read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI2);
}

/**
* @brief  Enables the write access to the FLASH.
* @param  None
* @retval : None
*/
void spi_flash_write_enable(void)
{
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Write Enable" instruction */
	spi_flash_send_byte(WREN);

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
}

/**
* @brief  Polls the status of the Write In Progress (WIP) flag in the
*   FLASH's status  register  and  loop  until write  opertaion
*   has completed.
* @param  None
* @retval 0: 成功
*        -1: 超时
*/
int spi_flash_wait_for_write_end(void)
{
	unsigned char FLASH_Status = 0;
	volatile unsigned int i = 0;
	/* Select the FLASH: Chip Select low */
	SPI_FLASH_CS_LOW();

	/* Send "Read Status Register" instruction */
	spi_flash_send_byte(RDSR);

	/* Loop as long as the memory is busy with a write cycle */
	do
	{
		/* Send a dummy byte to generate the clock needed by the FLASH
		and put the value of the status register in FLASH_Status variable */
		FLASH_Status = spi_flash_send_byte(Dummy_Byte);
		i++;
		if (i > 0xFFFFFFFE)	//测试超时
		{
			/* Deselect the FLASH: Chip Select high */
			SPI_FLASH_CS_HIGH();
			return -1;
		}
	}
	while ((FLASH_Status & WIP_Flag) == SET); /* Write in progress */

	/* Deselect the FLASH: Chip Select high */
	SPI_FLASH_CS_HIGH();
	//spi_flash_busy_flag = 0;
	return 0;
}

//返回SPI FLASH中FAT文件管理的容量,以sector为单位
int get_spi_flash_capacity(void)
{
	//return flasize/512;
	return FAT_FLASH_SIZE/512;			//只开放2M的空间给到FAT文件系统使用，SPI FLASH的实际容量还是由flasize指明
}

//将缓存的数据写入SPI FLASH
int spi_flash_post_write(void)
{
	if (current_cache_block == -1)
	{
		return 0;
	}
	if (current_cache_block > flasize/4096)
	{
		return -1;
	}

	if(spi_flash_erasesector(current_cache_block*4096)){	
		return -1;		//擦除失败
	}

	if (spi_flash_waddr(current_cache_block*4096, 4096, buffer))
	{
		return -2;		//写失败
	}

	current_cache_block = -1;

	return 0;
}

//此函数一次写1或者多个Sector（512字节）的数据
int spi_flash_write(unsigned int sector_offset,unsigned char *pBuffer,unsigned int sector_cnt)
{
	int i,tmp;
	unsigned int	cnt;
      
	if (sector_offset >= (flasize/512)){
		return -1;	//起始偏移超出容量
	}

	cnt = sector_cnt;
	if ((sector_offset + cnt) > (flasize/512))
	{
		cnt = (flasize/512) - sector_offset;
	}

	for (i = 0; i < cnt;i++)
	{
		if (((sector_offset+i)/8) == current_cache_block)
		{
			//如果当前要写的Sector位于已经被缓存的Block（4K Block）,那么直接将要写的数据写入缓存即可
			MEMCPY(buffer+((sector_offset+i)%8)*512,pBuffer+512*i,512);
		}
		else
		{
			//需要先将被缓存的数据写入SPI FLASH
			if(spi_flash_post_write())
			{
				return -2;
			}

			//将要写入的Sectro所在的Block全部读入缓存
			if(spi_flash_rpage((sector_offset+i)&0xfffffff8,8,buffer)){
				return -3;		//读取失败
			}

			//往缓存写入新的数据
			tmp = (sector_offset+i)%8; 
			MEMCPY(buffer+tmp*512,pBuffer+i*512,512);
			current_cache_block = (sector_offset+i)/8;	//标记当前缓存的Block
		}
	}
	

	return 0;
}

//此函数一次读1或者多个Sector（512字节）的数据
int spi_flash_read(unsigned int sector_offset,unsigned char *pBuffer,unsigned int sector_cnt)
{
	int i;//,tmp;
	unsigned int	cnt;

	if (sector_offset >= (flasize/512)){
		return -1;	//起始偏移超出容量
	}

	cnt = sector_cnt;
	if ((sector_offset + cnt) > (flasize/512))
	{
		cnt = (flasize/512) - sector_offset;
	}

	for (i = 0; i < cnt;i++)
	{
		if (((sector_offset+i)/8) == current_cache_block)
		{
			//如果当前要读的Sector位于已经被缓存的Block（4K Block）,那么直接读缓存数据即可
			MEMCPY(pBuffer+512*i,buffer+((sector_offset+i)%8)*512,512);
		}
		else
		{
			if(spi_flash_rpage(sector_offset+i,1,pBuffer+512*i)){
				return -3;		//读取失败
			}
		}
	}

	return 0;
}
/******************* (C) COPYRIGHT 2010 netcom *****END OF FILE****/
