/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : memory.c
* Author             : MCD Application Team
* Version            : V3.1.1
* Date               : 04/07/2010
* Description        : Memory management layer
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
#include "memory.h"
#include "usb_scsi.h"
#include "usb_bot.h"
#include "usb_regs.h"
#include "usb_mem.h"
#include "usb_conf.h"
#include "hw_config.h"
#include "mass_mal.h"
#include "usb_lib.h"
#include "usb_sil.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile u32 Block_Read_count = 0;
volatile u32 Block_offset;
volatile u32 Counter = 0;
u32  Idx;
//u32 Data_Buffer[BULK_MAX_PACKET_SIZE *2]; /* 512 bytes*/
u32 Data_Buffer[512]; /* 512 bytes*/
u8 TransferState = TXFR_IDLE;
/* Extern variables ----------------------------------------------------------*/
extern u8 Bulk_Data_Buff[BULK_MAX_PACKET_SIZE];  /* data buffer*/
extern u16 Data_Len;
extern u8 Bot_State;
extern Bulk_Only_CBW CBW;
extern Bulk_Only_CSW CSW;
extern u32 Mass_Memory_Size[2];
extern u32 Mass_Block_Size[2];

/* Private function prototypes -----------------------------------------------*/
/* Extern function prototypes ------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : Read_Memory
* Description    : Handle the Read operation from the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Read_Memory(u8 lun, u32 Memory_Offset, u32 Transfer_Length)
{
  static u32 Offset, Length;

  if (TransferState == TXFR_IDLE )
  {
    Block_Read_count = 0;		//20120830 joeÔö¼ÓµÄ
    Offset = Memory_Offset * Mass_Block_Size[lun];
    Length = Transfer_Length * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {
    if (!Block_Read_count)
    {
      MAL_Read(lun ,
               Offset ,
               Data_Buffer,
               Mass_Block_Size[lun]);

      USB_SIL_Write(EP1_IN, (u8 *)Data_Buffer, BULK_MAX_PACKET_SIZE);

      Block_Read_count = Mass_Block_Size[lun] - BULK_MAX_PACKET_SIZE;
      Block_offset = BULK_MAX_PACKET_SIZE;
    }
    else
    {
      USB_SIL_Write(EP1_IN, (u8 *)Data_Buffer + Block_offset, BULK_MAX_PACKET_SIZE);

      Block_Read_count -= BULK_MAX_PACKET_SIZE;
      Block_offset += BULK_MAX_PACKET_SIZE;
    }

    SetEPTxCount(ENDP1, BULK_MAX_PACKET_SIZE);
    SetEPTxStatus(ENDP1, EP_TX_VALID);
    Offset += BULK_MAX_PACKET_SIZE;
    Length -= BULK_MAX_PACKET_SIZE;

    CSW.dDataResidue -= BULK_MAX_PACKET_SIZE;
    //Led_RW_ON();
  }
  if (Length == 0)
  {
    Block_Read_count = 0;
    Block_offset = 0;
    Offset = 0;
    Bot_State = BOT_DATA_IN_LAST;
    TransferState = TXFR_IDLE;
    //Led_RW_OFF();
  }
}

/*******************************************************************************
* Function Name  : Write_Memory
* Description    : Handle the Write operation to the microSD card.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Write_Memory (u8 lun, u32 Memory_Offset, u32 Transfer_Length)
{

  static u32 W_Offset, W_Length;

  u32 temp =  Counter + BULK_MAX_PACKET_SIZE;

  if (TransferState == TXFR_IDLE )
  {
    W_Offset = Memory_Offset * Mass_Block_Size[lun];
    W_Length = Transfer_Length * Mass_Block_Size[lun];
    TransferState = TXFR_ONGOING;
  }

  if (TransferState == TXFR_ONGOING )
  {

    for (Idx = 0 ; Counter < temp; Counter++)
    {
      *((u8 *)Data_Buffer + Counter) = Bulk_Data_Buff[Idx++];
    }

    W_Offset += Data_Len;
    W_Length -= Data_Len;

    if (!(W_Length % Mass_Block_Size[lun]))
    {
      Counter = 0;
      MAL_Write(lun ,
                W_Offset - Mass_Block_Size[lun],
                Data_Buffer,
                Mass_Block_Size[lun]);
    }

    CSW.dDataResidue -= Data_Len;
    SetEPRxStatus(ENDP2, EP_RX_VALID); /* enable the next transaction*/   

    //Led_RW_ON();
  }

  if ((W_Length == 0) || (Bot_State == BOT_CSW_Send))
  {
    Counter = 0;
    Set_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
    TransferState = TXFR_IDLE;
    //Led_RW_OFF();
  }
}
#endif
/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

