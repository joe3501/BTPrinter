/**
*  @file 	Terminal_Para.h
*  @brief  	�������ն���Ҫ������NVM����Ĳ��������ݽṹ
*  @note	
*/

#ifndef _TERMINAL_PARA_H_
#define _TERMINAL_PARA_H_


/**
*@brief ����洢��SPI FLASH�ڲ��ն˲����Ľṹ����
*@ note	
*/
#pragma pack(1)

typedef struct  {
	unsigned int			checkvalue;					//4�ֽ�		0	B	�˷ݽṹ��У��ֵ crc32			
	unsigned int			line_after_mark;			//4�ֽ�		4	B
	unsigned int 			max_mark_length;			//4�ֽ�		8	B
	unsigned char 			character_code_page;		//1�ֽ�		12	B
	unsigned char			rfu[16+32-1];				//47�ֽ�	13  B
	unsigned char			struct_ver;					//1�ֽ�		60	B	�����汾�ţ���ʶ�˽ṹ�İ汾
	unsigned char			endtag[3];					//0x55,0xAA,0x5A  61      һ��64�ֽ�
} TTerminalPara;
#pragma pack()

extern TTerminalPara		g_param;				//�ն˲���

int ReadTerminalPara(void);
int SaveTerminalPara(void);
int DefaultTerminalPara(void);
#endif
