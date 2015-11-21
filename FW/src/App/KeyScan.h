#ifndef _KEYSCAN_H
#define _KEYSCAN_H

#define KEY_FEED()  ( GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2))

extern void KeyScanProc(void);
extern void KeyScanInit(void);


#endif

