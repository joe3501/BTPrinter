#ifndef _TP_H_
#define _TP_H_


//================================================
#if defined(PT1043)
#define LineDot		(832)
#elif defined(PT487_100DPI)
#define LineDot		(192)
#else
#define LineDot		(384)
#endif

//======================================================================================================
extern uint8_t clr_all_dot;
extern void TPInit(void);
extern void TPSelfTest(void);

extern void TPSetSpeed(uint8_t speed);
extern void TPFeedToMark(uint16_t line);
extern void TPFeed(int8_t direction);
extern void TPFeedStop(void);
extern void TPFeedLine(uint16_t line);
extern void TPPrintLine(uint8_t  *dot);
extern void TPFeedStart(void);
extern void TPFeedStop(void);
extern uint8_t TPPrinterReady(void);
extern uint8_t TPPrinterBusy(void);
extern void TPIntSetIdle(void);
extern uint8_t IsPrintBufEmpty(void);
extern void TPPrintTestPage(void);
extern void PrintBufPushBytes(uint8_t c);
extern void Wake_up(void);
extern void SetDesity(void);
extern uint8_t IsPrinterIdle(void);
extern void TPSelfTest2(void);
extern void TIM3_IRQ_Handle(void);
extern void test_motor(void);
//======================================================================================================


#endif






