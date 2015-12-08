/**
* @file main.c
* @brief ������ӡ����Ŀ������
*
* @version V0.0.1
* @author joe
* @date 2015��11��12��
* @note
*		none
*
* @copy
*
* �˴���Ϊ���ںϽܵ������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*/

/* Private Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include <string.h>
#include <stdlib.h>
#include "hw_platform.h"
#include "data_uart.h"
#include "TimeBase.h"
#include "Terminal_para.h"
#include "usb_app_config.h"
#include "res_spi.h"
#include "Event.h"
/* Private define ------------------------------------------------------------*/

// Cortex System Control register address
#define SCB_SysCtrl					((u32)0xE000ED10)
// SLEEPDEEP bit mask
#define SysCtrl_SLEEPDEEP_Set		((u32)0x00000004)

#ifdef DEBUG_VER
unsigned short	debug_buffer[8000];
unsigned int	debug_cnt;
#endif


/* Global variables ---------------------------------------------------------*/
ErrorStatus			HSEStartUpStatus;							//Extern crystal OK Flag


/* Private functions ---------------------------------------------------------*/
static void Unconfigure_All(void);
static void GPIO_AllAinConfig(void);
void RCC_Configuration(void);

/* External variables -----------------------------------------------*/
extern	TTerminalPara			g_param;					//Terminal Param

/*******************************************************************************
* Function Name  : system_error_tip
* Description    : ���ص�ϵͳ������ʾ
*******************************************************************************/
void system_error_tip(void)
{
	//@todo...

	while(1);
}

/*******************************************************************************
* Function Name  : enter_u_disk_mode
* Description    : ����U��ģʽ
*******************************************************************************/
void enter_u_disk_mode(void)
{
	int key_state = 0;
	g_mass_storage_device_type = MASSTORAGE_DEVICE_TYPE_SPI_FLASH;
	usb_device_init(USB_MASSSTORAGE);
	//usb_Cable_Config(ENABLE);
	
	while(1)
	{
		delay_ms(50);
		if(!KEY_FEED())
		{
			key_state++;
		}
		else
		{
			key_state = 0;
		}
		if( key_state == 20)		//�˳�USBģʽ��ϵͳ��λ
		{
			//usb_Cable_Config(DISABLE);
			NVIC_SETFAULTMASK();
			NVIC_GenerateSystemReset();
		}
	}
}

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
	int	ret,key_state;
	/* System Clocks Configuration **********************************************/
	RCC_Configuration(); 
#ifdef RELEASE_VER
	/* NVIC Configuration *******************************************************/
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, IAP_SIZE);		//��Ҫ���ܵ� bootcode
#else	
	/* NVIC Configuration *******************************************************/
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
#endif

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);		//��������ϵͳ�����ȼ�����ΪGroup1��1λ��ռ���ȼ���2��������ж�Ƕ�ף�	3λ��Ӧ���ȼ�
	
	// Clear SLEEPDEEP bit of Cortex System Control Register
	*(vu32 *) SCB_SysCtrl &= ~SysCtrl_SLEEPDEEP_Set;

	Unconfigure_All();
	// ���ݴ���(���Կ�)��ʼ��
	data_uart_init();
       
        //while(1);

#ifdef DEBUG_VER
	memset(debug_buffer,0,8000*2);
	debug_cnt = 0;

	printf("BTPrinter startup...\r\n");
	if (HSEStartUpStatus == SUCCESS)
	{
		printf("HSE OK!\r\n");
	}
	else
	{
		printf("HSE Failed!\r\n");
	}
#endif

	//��ʼ��ʱ������
	TimeBase_Init();


	//��ʼ������ģ��
	KeyScanInit();

	key_state = 0;
	if (!KEY_FEED())
	{
		key_state = 1;
	}
#ifdef HW_VER_LCD
	//��ʼ��LCD
	//@todo...		//for debug
#else
	//��ʼ��LED
	//@todo...
	delay_ms(10);	//for debug
#endif

	if (!KEY_FEED())
	{
		if (key_state == 1)
		{
			key_state = 2;
		}
	}
 
	//��ʼ������ģ�飨SPI FLASH��
	ret = ReadTerminalPara();
	if (ret)
	{
		if (ret > 0 || ret == -4)
		{
			if (DefaultTerminalPara())
			{
				system_error_tip();	//���صĴ���ϵͳֹͣ�������У����ܽ������ģʽ
			}
		}
		else
		{
			system_error_tip();	//���صĴ���ϵͳֹͣ�������У����ܽ������ģʽ
		}
	}

	event_init();
	
	//���ϵͳ���ֿ���Դ�Ƿ���ȷ
	res_upgrade();
	ret = res_init();
	if (ret != 0)
	{
		enter_u_disk_mode();
	}
	else
	{
		if (key_state == 2)
		{
			if (usb_cable_insert())
			{
				enter_u_disk_mode();
			}
			else
			{
				event_post(evtKeyDownHold2000msMode);
			}
		}
	}

	//��ʼ��������ӡͷ�Ŀ���IO����ʱ��
	print_head_init();

	//��ʼ������ģ��
	BT816_init();

	esc_init();

	usb_device_init(USB_PRINTER);
	//test_motor();

	PaperStartSns();		//Systick��������


	while (1)
	{
		esc_p();		//�ȴ����ڽ��յ������ݣ����ݲ�ͬ�����ݽ�����Ӧ�Ĵ���
	}
}

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{   
	vu32 i=0;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE							*/
	RCC_HSEConfig(RCC_HSE_ON);
	// ����Ҫ����ʱ�����ܼ���ĳЩ�Ƚϲ�ľ��壬�Ա�˳������	
	for(i=0; i<200000; i++);

	/* Wait till HSE is ready			*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer		*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state			*/
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK					*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 

		/* PCLK2 = HCLK					*/
		RCC_PCLK2Config(RCC_HCLK_Div1); 

		/* PCLK1 = HCLK/2					*/
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* PLLCLK = 12MHz * 6 = 72 MHz	*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

		/* PLLCLK = 8MHz * 9 = 72 MHz	*/
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

		/* Enable PLL						*/
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready		*/
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
}

/*******************************************************************************
* Function Name  : Unconfigure_All
* Description    : set all the RCC data to the default values 
*                  configure all the GPIO as input
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Unconfigure_All(void)
{
	//RCC_DeInit();

	/* RCC configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALL, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_ALL, DISABLE);

	GPIO_AllAinConfig();
}


/*******************************************************************************
* Function Name  : GPIO_AllAinConfig
* Description    : Configure all GPIO port pins in Analog Input mode 
*                  (floating input trigger OFF)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void GPIO_AllAinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/************************************************
* Function Name  : EnterLowPowerMode()
************************************************/
void EnterLowPowerMode(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//BT816_enter_sleep();
	stop_real_timer();

	//������Ҫ����PWRģ���ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // Enable PWR clock

	// enable Debug in Stop mode
	//DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
	
	//����͹���ģʽ
	EXTI_ClearFlag(0xffff);
	PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}

/************************************************
* Function Name  : ExitLowPowerMode()
************************************************/
void ExitLowPowerMode(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//��������ʱ��
	RCC_Configuration();
	//BT816_wakeup();
	start_real_timer();
}

//�ر�ĳһ���ж�
 void NVIC_DisableIRQ(unsigned char	irq_channel)
{
	NVIC_InitTypeDef							NVIC_InitStructure;
	/* Enable the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= irq_channel;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//ʹ��ĳһ���ж�
 void NVIC_EnableIRQ(unsigned char	irq_channel)
{
	NVIC_InitTypeDef							NVIC_InitStructure;
	/* Enable the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= irq_channel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/

//void assert_failed(u8* file, u32 line)
//{ 
/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

/* Infinite loop */
//while (1)
//{
//}
//}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

