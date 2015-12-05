#include "Type.h"
#include "Event.h"
#include "TP.h"
#include "basic_fun.h"
#include "Esc_p.h"
#include "ESC_POS_test.h"


//======================================================================================================
static volatile uint8_t evtHead;
static uint8_t evtTail;
static uint8_t evtQueue[64];
//======================================================================================================

//======================================================================================================
extern void event_init(void)
{
	evtHead = evtTail = 0;
}
//======================================================================================================
extern void event_post(uint8_t event)
{
	uint32_t head;

	if(event)
	{
         // Disable Interrupts
		head = evtHead;
		evtHead = (head + 1) & (ARRAY_SIZE(evtQueue) - 1);
		// restore flag

		evtQueue[head] = event;
	}
}
//======================================================================================================
//======================================================================================================
extern uint8_t event_pend(void)
{
	uint8_t event;

	if(evtHead == evtTail)
	{
		return evtNULL;
	}
	event = evtQueue[evtTail];
	evtTail = (evtTail + 1) & (ARRAY_SIZE(evtQueue) - 1);
	return event;
}
//======================================================================================================
extern void event_proc(void)
{
	int i;
	switch(event_pend())
	{
	//-----------------------------------------------------------------
	case evtKeyDownFeed:
		//ESC_POS_test_esc();
		//ESC_POS_test_esc_special();
		//TPPrintTestPage();
		TPFeedStart();
		//TPSelfTest2();
		break;
	case evtKeyUpFeed:
		TPFeedStop();
		break;
	case evtKeyDownHold500msMode:
        break;
	case evtKeyDownMode:
		#if 1
		if(TPPrinterReady())
        TPPrintTestPage();
		#endif
		break;
	case evtKeyDownHold2000msMode:
		if(TPPrinterReady())
        TPPrintTestPage();
		break;
	case evtKeyUpMode:
		break;
	case evtKeyHoldMode:
		break;
	case evtKeyDownHold5000msMode:
		break;
    case evtKeyDownHold7000msMode:
        break;
	case evtPaperOut:
		for (i = 0; i< MAX_PT_CHANNEL;i++)
		{
			esc_sts[i].status4 |= (0x03<<5);
		}
        
		break;
	case evtPaperIn:
		for (i = 0; i< MAX_PT_CHANNEL;i++)
		{
			esc_sts[i].status4 &= ~(0x03<<5);
		}
        Wake_up();
        break;
    case evtBmDetect:
        break;
    case evtGetRealTimeStatus4:
       //Putchar(esc_sts.status4);
        break;
	case evtLifetest:
	   break;
	default:
		break;
	}
}
//======================================================================================================




