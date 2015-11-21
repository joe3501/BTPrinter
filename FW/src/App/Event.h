#ifndef __EVENT_H__
#define __EVENT_H__
//======================================================================================================
enum
{
	evtNULL = 0,
	evtKeyDownMode,
	evtKeyUpMode,
	evtKeyHoldMode,
	evtPaperOut,
	evtPaperIn,
	evtKeyDownHold500msMode,
	evtKeyDownHold2000msMode,
	evtKeyDownHold5000msMode,
	evtKeyDownHold7000msMode,

	evtKeyUpFeed,
	evtKeyDownFeed,
	evtKeyDownHold500msFont,
	evtKeyDownHold2000msFont,
	evtKeyDownHold5000msFont,
	evtKeyDownHold7000msFont,
    evtBmDetect,
    evtBmNotDetect,
    evtGetRealTimeStatus4,
    evtLifetest,
	evtMax
};

//======================================================================================================
extern void event_init(void);
extern void event_post(uint8_t event);
extern uint8_t event_pend(void);
extern void event_print(void);
extern void event_proc(void);

//======================================================================================================
#endif


