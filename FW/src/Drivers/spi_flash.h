/**
***************************************************************************
* @file  spi_flash.h
* @brief SPI Flash 底层驱动头文件
***************************************************************************
*
* @version V0.0.1
* @author jiangcx
* @date 2010年09月06日
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_
#include "stm32f10x_lib.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

//CS  -- PD.10
#define GPIO_CS                  GPIOD
#define RCC_APB2Periph_GPIO_CS   RCC_APB2Periph_GPIOD
#define GPIO_Pin_CS              GPIO_Pin_10
#define SPI_FLASH_PageSize		 0x100
#define SPI_FLASH_PageMsk		 0xFFFFFF00

/* Exported macro ------------------------------------------------------------*/
/* Select SPI FLASH: Chip Select pin low  */
#define SPI_FLASH_CS_LOW()       GPIO_ResetBits(GPIO_CS, GPIO_Pin_CS)
/* Deselect SPI FLASH: Chip Select pin high */
#define SPI_FLASH_CS_HIGH()      GPIO_SetBits(GPIO_CS, GPIO_Pin_CS)


extern unsigned int			fatfs_sector_offset;
/* Exported functions ------------------------------------------------------- */
/*----- High layer function -----*/
int		spi_flash_init(void);
void	spi_flash_close(void);

int		spi_flash_valid(void);
unsigned int	spi_flash_read_id(void);

int		spi_flash_erasesector(unsigned int SectorAddr);
int		spi_flash_eraseblock(unsigned int BlockAddr);
int		spi_flash_erase(void);

int		spi_flash_rpage(int lba, int sector, unsigned char *buf);
int		spi_flash_wpage(unsigned int WriteAddr, unsigned char *pBuffer);

int		spi_flash_waddr(unsigned int WriteAddr, unsigned int NumByteToWrite, unsigned char *pBuffer);
void	spi_flash_raddr(unsigned int ReadAddr,unsigned int NumByteToRead, unsigned char *pBuffer);
int		spi_flash_wait_for_write_end(void);
int		get_spi_flash_capacity(void);
int		spi_flash_post_write(void);
int		spi_flash_write(unsigned int sector_offset,unsigned char *pBuffer,unsigned int sector_cnt);
int		spi_flash_read(unsigned int sector_offset,unsigned char *pBuffer,unsigned int sector_cnt);
#endif /* __SPI_FLASH_H */

/******************* (C) COPYRIGHT 2010 netcom *****END OF FILE****/
