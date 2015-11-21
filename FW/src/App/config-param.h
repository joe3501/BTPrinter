#ifndef _CONFIG_PARAM_H_
#define _CONFIG_PARAM_H_

//#define Font_Size 1024UL
#define Font_Size 8UL

#if defined(FONT_DOWNLOAD)
typedef struct
{
    uint32_t        font_head;
    uint32_t        pack;
    uint16_t        len;
    uint8_t         font[Font_Size];
    uint16_t        cksum;
}FONT_T;

typedef struct
{
    uint32_t        font_head;
    uint32_t        pack;
    uint16_t        ack;
    uint16_t        cksum;
}FONT_ACK_T;
#endif

typedef struct
{
	unsigned long id_head;
    unsigned short int line_after_mark;
    unsigned short int max_mark_length;
    unsigned char print_voltage;
	unsigned char com_baud;
    unsigned char character_code_page;//11
	unsigned char reserved[32-11-2];
	unsigned short int cksum;

}PARA_T;
extern  uint16_t Flash_ID;
extern void font_download_mode(void);
extern uint32_t config_idx2baud(uint8_t idx);
extern void UpdateParaSector(PARA_T * para_b);
extern void config_init(void);
extern void config_mode(void);
extern void EraseConfig(void);


extern PARA_T para;
extern uint16_t Flash_ID;
extern uint32_t bitmap_addr;
extern uint8_t font_download;

//----chang
#if defined(CODEPAGE)
extern uint32_t font_en_addr;
#endif


#endif

