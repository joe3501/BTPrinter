/****************************************************************************
 *   $Id:: uart.h 3635 2010-06-02 00:31:46Z usb00423                        $
 *   Project: NXP LPC11xx software example
 *
 *   Description:
 *     This file contains definition and prototype for UART configuration.
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
#ifndef __UART_H
#define __UART_H
#include "BT816.h"
#include "ringbuffer.h"

#define MAX_PRINT_CHANNEL		(MAX_BT_CHANNEL+1)		//BT channel + USB channel

#define USB_PRINT_CHANNEL_OFFSET		(MAX_BT_CHANNEL)

extern unsigned char		spp_rec_buffer[MAX_BT_CHANNEL][SPP_BUFFER_LEN];
extern struct ringbuffer	spp_ringbuf[MAX_PRINT_CHANNEL];
extern unsigned char		usb_rec_buffer[USB_BUFFER_LEN];
//extern uint8_t Putchar(uint8_t c) ;
extern uint8_t Getchar(void);
extern void PrintBufPushLine( uint8_t *p, uint32_t len);
extern uint8_t PrintBufGetchar(uint8_t *ch);
extern void WakeUpTP_MODE1(void);
extern void PrintBufToZero(void);

#endif /* end __UART_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
