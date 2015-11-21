#ifndef _CRC32_H_
#define _CRC32_H_

void BuildTable32( unsigned long aPoly );
//unsigned long CRC_32( unsigned char * aData, unsigned long aSize );
unsigned long crc32(unsigned long crc,unsigned char *buf,int len);

#endif
