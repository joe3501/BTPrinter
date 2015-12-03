#ifndef _RES_SPI_H_
#define _RES_SPI_H_


// �ֵ�ID����
//#define FNT_ASC_6_12				0
#define FNT_ASC_8_16				0
#define FNT_ASC_12_24				1
#define FNT_CHN_16_16				2
#define FNT_CHN_24_24				3

typedef struct {
	unsigned char					type;				// ������˵��
	unsigned char					width;				// �ֿ�
	unsigned char					height;				// �ָ�
	unsigned char					size;				// һ����ģռ���ֽ���
	unsigned int					address;			// �����ROM�ֿ⣬��Ϊ�ڴ�ָ��
	// �������Դ�ֿ⣬��Ϊ��ʼ����
}TFontInfo;

extern	TFontInfo FontList[];

int res_upgrade(void);
int res_init(void);
unsigned char *read_resdata(unsigned int dwLBA);
void font_data_read(unsigned char font_type,unsigned char *c,unsigned char *pBuf,unsigned int size);
#endif
