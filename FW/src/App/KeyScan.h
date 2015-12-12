#ifndef _KEYSCAN_H
#define _KEYSCAN_H

#define KEY_FEED()  ( GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2))

void KeyScanProc(void);
void KeyScanInit(void);
void box_ctrl(int ms);

#endif

