#ifndef _LED_H
#define _LED_H



#define Led_Paper_ON()     do{ \
           LPC_GPIO0->MASKED_ACCESS[0x80] = 0x80; \
           }while(0)

#define Led_Paper_OFF()    do{ \
           LPC_GPIO0->MASKED_ACCESS[0x80] = 0x00; \
           }while(0)


extern void LedInit(void);
extern void LedScanProc(void);

#endif
