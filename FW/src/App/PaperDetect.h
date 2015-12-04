#ifndef __PAPERDETECT_H__
#define __PAPERDETECT_H__

extern uint16_t		AD_Value[10][2];		//分别存放缺纸检测的AD值和温度检测的AD值

#define TEMP_SNS_OFFSET		0
#define	PAPER_SNS_OFFSET	1

extern uint8_t	TPPrinterMark(void);
extern void		PaperPlatenSNSInit(void);
extern uint8_t	TPPrinterReady(void);
extern uint8_t TPPrinterReady_ext(void);

extern void		TPPaperSNSDetect(uint8_t c);

extern void		TPPaperSNSInit(void);

extern void		PaperStartSns(void);

extern uint8_t	TPPaperReady(void);
extern void		SysTick_IRQ_Handle(void);
#endif


