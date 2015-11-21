#include "includes.h"

//==========================led=============================

extern void LedInit(void)
{
	LPC_IOCON->PIO0_7=0x0;
    GPIOSetDir(PORT0,GPIO_Pin_7,Output);
	Led_Paper_OFF();

}

static int8_t led_flash_cnt;
#define LED_SLOW_FLASH_SPEED    4
#define LED_FAST_FLASH_SPEED    60

extern void LedScanProc(void)
{
	if(TPPrinterReady() == 0)	// 打印机未就绪
	{
	        if(TPPaperReady() == 0 )//缺纸
	        {
	            if(led_flash_cnt > 0)
	        	{
	        		if((--led_flash_cnt) == 0)
	        		{
	        			Led_Paper_OFF();
	        			led_flash_cnt = -LED_FAST_FLASH_SPEED;
	        		}
	        	}
	        	else if(led_flash_cnt < 0)
	        	{
	        		if((++led_flash_cnt) == 0)
	        		{
	        			Led_Paper_ON();
	        			led_flash_cnt = LED_FAST_FLASH_SPEED;
	        		}
	        	}
	        	else
	        	{
	        		Led_Paper_ON();
	        		led_flash_cnt = LED_FAST_FLASH_SPEED;
	        	}
	        }
		else
		{
			Led_Paper_OFF();
		}
	}
	else	// 打印机就绪
	{
		Led_Paper_OFF();
        led_flash_cnt = 0;
	}
}














