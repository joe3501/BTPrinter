#ifndef __FONT_H__
#define __FONT_H__

//==========================================================
#define ANTITYPE (0)
#define CIR_NINETY_DEGREE (1)
#define CIR_TWO_NINETY_DEGREE (2)
#define CIR_THREE_NINETY_DEGREE (3)


//======================================================================================================
enum
{
	CHINESE_FONT_GB2312,
    CHINESE_FONT_GBK,
	CHINESE_FONT_GB13000,
	CHINESE_FONT_GB18030,
	CHINESE_FONT_MAX
};

extern void FontEnlarge(uint8_t  *dot, uint8_t  *buf, uint8_t row);
extern void FontUnderline(uint16_t start_col, uint16_t end_col);
extern void GetEnglishFont(uint8_t ascii);
extern void GetChineseFont(uint8_t  *c, uint8_t charset);
extern void GetEnglishHRIFont(uint8_t ascii);

//======================================================================================================
#endif

