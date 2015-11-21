/****************************************************************************
 *   $Id:: uart.c 3648 2010-06-02 21:41:06Z usb00423                        $
 *   Project: NXP LPC11xx UART example
 *
 *   Description:
 *     This file contains UART code example which include UART
 *     initialization, UART interrupt handler, and related APIs for
 *     UART access.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include "ringbuffer.h"
#include "BT816.h"
#include "Event.h"
/******************************************************************************
**Function name:  Getchar
**
**Description:  Get a char from the uart 0 port
**
**parameters: none
**Returned value:
**
******************************************************************************/
extern uint8_t Getchar(void)        //接收数据
    {
        uint8_t c;
        while(1)
        {
            event_proc();

			if (ringbuffer_getchar(&spp_ringbuf,&c))
			{
				return c;
			}
        }

    }

 extern uint8_t PrintBufGetchar(uint8_t *ch)
 {
	return ringbuffer_getchar(&spp_ringbuf,ch);
 }

 extern void PrintBufPushBytes(uint8_t c)
 {
      ringbuffer_putchar(&spp_ringbuf,c);
 }

//======================================================================================================
extern void PrintBufPushLine( uint8_t *p, uint32_t len)
{
	ringbuffer_put(&spp_ringbuf,p,len);
}
extern void PrintBufToZero(void)
{
     ringbuffer_reset(&spp_ringbuf);
}
/******************************************************************************
**                            End Of File
******************************************************************************/
