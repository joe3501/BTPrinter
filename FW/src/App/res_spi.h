#ifndef _RES_SPI_H_
#define _RES_SPI_H_


// 字的ID定义
//#define FNT_ASC_6_12				0
#define FNT_ASC_8_16				0
#define FNT_ASC_12_24				1
#define FNT_CHN_16_16				2
#define FNT_CHN_24_24				3

typedef struct {
	unsigned char					type;				// 见下面说明
	unsigned char					width;				// 字宽
	unsigned char					height;				// 字高
	unsigned char					size;				// 一个字模占的字节数
	unsigned int					address;			// 如果是ROM字库，则为内存指针
	// 如果是资源字库，则为起始扇区
}TFontInfo;

extern	TFontInfo FontList[];

int res_upgrade(void);
int res_init(void);
unsigned char *read_resdata(unsigned int dwLBA);
void font_data_read(unsigned char font_type,unsigned char *c,unsigned char *pBuf,unsigned int size);
#endif
