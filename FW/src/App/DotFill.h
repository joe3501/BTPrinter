#ifndef __DOTFILL_H__
#define __DOTFILL_H__
//======================================================================================================
extern void DotFillToBuf(uint8_t  *buf, uint16_t col, uint8_t row, uint8_t underline);
extern void DotBufFillToPrn(uint8_t  *buf, uint16_t max_col, uint16_t max_rowbyte, uint16_t min_row, uint16_t max_row, uint8_t ratio_width, uint8_t ratio_height);
extern void PrintCurrentBuffer(uint16_t n);
extern void BufFillToPrn(uint16_t n);
extern void  PictureDotFillToBuf(uint8_t *buf, uint16_t col, uint16_t row);
extern void BufFillToPrn_0(uint16_t n);
extern void PrintCurrentBuffer_0(uint16_t n);
extern uint16_t max_start_col;
//======================================================================================================
#endif

