#include "Type.h"
#include "KeyScan.h"
#include "Event.h"
#include "stm32f10x_lib.h"

#define KEY_MODE_DB_SHIFT		(1UL << 0)
#define KEY_MODE_SHIFT			(1UL << 1)
#define KEY_FEED_DB_SHIFT		(1UL << 2)
#define KEY_FEED_SHIFT			(1UL << 3)


static uint8_t keyStatus;

extern void KeyScanProc(void)
{
	if(KEY_FEED())		// FEED key up
	{
		if((keyStatus & KEY_FEED_DB_SHIFT) == 0)	// status no changed
		{
			if(keyStatus & KEY_FEED_SHIFT)	// previous mode key up
			{
				event_post(evtKeyUpFeed); // mode key up
				keyStatus &= ~KEY_FEED_SHIFT;
			}
		}
		else
		{
			keyStatus &= ~KEY_FEED_DB_SHIFT;//keyStatus = xxx0
		}
	}
	else
	{
		if(keyStatus & KEY_FEED_DB_SHIFT)	// status no changed
		{
			if((keyStatus & KEY_FEED_SHIFT) == 0)	// previous mode key down
			{
				event_post(evtKeyDownFeed);	// mode key down
				keyStatus |= KEY_FEED_SHIFT;//
			}
		}
		else
		{
			keyStatus |= KEY_FEED_DB_SHIFT;//keyStatus =xxx1
		}
	}

	//for burning test,auto post key event
        if (IsPrinterFree())
        {
			event_post(evtKeyDownFeed);
        }
        
}
/*
static void PowerOnSelfTest(void)
{
	// two keys down
	//if((keyStatus & (KEY_FEED_SHIFT | KEY_MODE_SHIFT)) == (KEY_FEED_SHIFT | KEY_MODE_SHIFT))
	if((keyStatus & (KEY_FEED_SHIFT)) == (KEY_FEED_SHIFT ))
	{
		event_post(evtSelfTest);
	}
	systimeRemoveCallback(PowerOnSelfTest);
}
*/

//按键初始化
void KeyScanInit(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;

	//FEED_KEY	-- PE.2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//BOX_CTRL	-- PA.4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);

	keyStatus = 0;
}


//开启钱箱控制的脉冲
void box_ctrl(int ms)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	delay_ms(ms);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}




