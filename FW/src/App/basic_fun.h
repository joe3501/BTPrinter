#ifndef BASIC_FUN_H
#define BASIC_FUN_H

#define ARRAY_SIZE(arr)		(sizeof(arr) / sizeof(arr[0]))
#define ALIGN_SIZE		4

#define KT_TINY_SIZE		//小内存模式

#define	MEMSET		kt_memset
#define	MEMCPY		kt_memcpy
#define	MEMCMP		kt_memcmp
#define	MEMMOVE		kt_memmove
#define	STRSTR		kt_strstr
#define	STRCASECMP	kt_strcasecmp
#define	STRNCPY		kt_strncpy
#define	STRCMP		kt_strcmp
#define	STRLEN		kt_strlen

/**
 * @ingroup BasicDef
 *
 * @def ALIGN(size, align)
 * Return the most contiguous size aligned at specified width. RT_ALIGN(13, 4)
 * would return 16.
 */
#define ALIGN(size, align)           (((size) + (align) - 1) & ~((align) - 1))

/**
 * @ingroup BasicDef
 *
 * @def ALIGN_DOWN(size, align)
 * Return the down number of aligned at specified width. RT_ALIGN_DOWN(13, 4)
 * would return 12.
 */
#define ALIGN_DOWN(size, align)      ((size) & ~((align) - 1))



void itoascii(unsigned int int_value, unsigned char *pValue, unsigned int *plen);
//void int_to_ascii(unsigned int int_value, unsigned char *pValue, unsigned int len, unsigned char type);
unsigned char hex_to_str(unsigned int hex_val, unsigned char type, unsigned char outlen, unsigned char *outstr);
int HexToBCD(unsigned int indata,unsigned char *outdata,unsigned char out_len);
int BCDToHex(unsigned char* indata,unsigned char len,unsigned int *outdata);
unsigned int bcd_field_to_str(unsigned char *bcd_field,unsigned int len,unsigned char *out);
void str_to_bcd_field(unsigned char *str,unsigned char *bcd_field,unsigned int len);
unsigned char HexToAscii(unsigned char hex);
unsigned char Ascii_To_Hex(unsigned char  asciiValue);
int ascii_to_int(unsigned char *pValue);

int	unicode_to_gb2312(unsigned char *pUnicode, unsigned char *pGB2312, int len);

//void ascii_to_keycode(unsigned char *pAscii, unsigned char *pKeycode, unsigned int len);
void ascii_to_keyreport(unsigned char ascii, unsigned char*report);
void ascii_to_keyreport2(unsigned char ascii,unsigned char*report);
unsigned char ascii_to_keyreport2_ext(unsigned char *ascii,unsigned char len,unsigned char*report);

int binary_search(unsigned short *search_list, unsigned int len, unsigned short key);
void build_time_dis_str(unsigned char *date_time_bcd,unsigned char *dis_str,unsigned char mode);

char *F2S(float d, char* str);
int if_last_char_valid(unsigned char *buf,unsigned int len);
int	if_include_unicode_character(unsigned char *buf,unsigned int len);

void *kt_memset(void *s, int c, unsigned int count);
void *kt_memcpy(void *dst, const void *src, unsigned int count);
void *kt_memmove(void *dest, const void *src, unsigned int n);
unsigned int kt_memcmp(const void *cs, const void *ct, unsigned int count);
char *kt_strstr(const char *s1, const char *s2);
unsigned int kt_strcasecmp(const char *a, const char *b);
char *kt_strncpy(char *dst, const char *src, unsigned int n);
unsigned int kt_strncmp(const char *cs, const char *ct, unsigned int count);
unsigned int kt_strcmp(const char *cs, const char *ct);
unsigned int kt_strlen(const char *s);

#endif //BASIC_FUN_H