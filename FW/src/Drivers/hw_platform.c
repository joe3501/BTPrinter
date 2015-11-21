/**
* @file hw_platform.c
* @brief H520B Ӳ����ؽӿڵ�ʵ��
* @version V0.0.1
* @author joe.zhou
* @date 2015��08��31��
* @note
* @copy
*
* �˴���Ϊ���ںϽܵ������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
* ����˾�������Ŀ����˾����һ��׷��Ȩ����
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*
*/
#include "hw_platform.h"
#include "stm32f10x_lib.h"
#include "ucos_ii.h"
#include "TimeBase.h"
#include <assert.h>

static unsigned int	charge_state_cnt;
static unsigned int	last_charge_detect_io_cnt;

unsigned int	charge_detect_io_cnt;

static unsigned char	current_led_state;
static VTIMER_HANDLE	led_timer_h[4];

#define		LED_RED_MASK			(0x01<<0)
#define		LED_GREEN_MASK			(0x01<<1)
#define		LED_YELLOW_MASK			(0x01<<2)
#define		LED_BLUE_MASK			(0x01<<3)

#define		LED_RED_ON_MASK			(0x01<<4)
#define		LED_GREEN_ON_MASK		(0x01<<5)
#define		LED_YELLOW_ON_MASK		(0x01<<6)
#define		LED_BLUE_ON_MASK		(0x01<<7)

//USB_CHK		PA1	  

//LED_RED		PA4   
//LED_GREEN		PA5   
//LED_YELLOW	PA6   
//MOTOR			PA7   

//ADC_IN		PB0	  

//BEEP			PB5   
//CHARGE		PB6	  
//RFU_IO1		PB7
//RFU_IO2		PB8

//TRIG			PB12

/**
* @brief	��ʼ�����ڵ�ص�ѹ����ADCģ��
* @param[in]  none
* @param[out] none
* @return     none
* @note                    
*/
static void ADC_Module_Init(void)
{
	ADC_InitTypeDef   adc_init;
	GPIO_InitTypeDef  gpio_init;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	//PB.0   ADC-IN
	gpio_init.GPIO_Pin  = GPIO_Pin_0;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOB, &gpio_init);

	adc_init.ADC_Mode               = ADC_Mode_Independent;		//
	adc_init.ADC_ScanConvMode       = DISABLE;
	adc_init.ADC_ContinuousConvMode = ENABLE;
	adc_init.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	adc_init.ADC_DataAlign          = ADC_DataAlign_Right;
	adc_init.ADC_NbrOfChannel       = 1;
	ADC_Init(ADC1, &adc_init);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_13Cycles5);
	ADC_Cmd(ADC1, ENABLE);

	// ������ADC�Զ�У׼����������ִ��һ�Σ���֤����
	// Enable ADC1 reset calibaration register 
	ADC_ResetCalibration(ADC1);
	// Check the end of ADC1 reset calibration register
	while(ADC_GetResetCalibrationStatus(ADC1));

	// Start ADC1 calibaration
	ADC_StartCalibration(ADC1);
	// Check the end of ADC1 calibration
	while(ADC_GetCalibrationStatus(ADC1));
	// ADC�Զ�У׼����---------------

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);			//��ʼת��
}

/**
* @brief  Initialize the IO
* @return   none
*/
static void platform_misc_port_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	//USB_CHK -- PA.1
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Connect EXTI Line6 to PB.6 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

	EXTI_ClearITPendingBit(EXTI_Line1);
	EXTI_InitStructure.EXTI_Line = EXTI_Line1;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
	EXTI_GenerateSWInterrupt(EXTI_Line1);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);      
	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//ChargeState detect -- PB.6
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Connect EXTI Line6 to PB.6 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource0);

	EXTI_ClearITPendingBit(EXTI_Line6);
	EXTI_InitStructure.EXTI_Line = EXTI_Line6;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure); 
	EXTI_GenerateSWInterrupt(EXTI_Line6);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);      
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	charge_detect_io_cnt = 0;
	last_charge_detect_io_cnt = 1;

	//LED-Red -- PA.4	LED-Green -- PA.5	LED-Yellow -- PA.6		Motor -- PA.7
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6);
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);

	//Beep -- PB.5	LED-Blue -- PB.7
	GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,  GPIO_Pin_7);
	GPIO_ResetBits(GPIOB, GPIO_Pin_5);

	//RFU-IO2 -- PB.8		
	//GPIO_InitStructure.GPIO_Pin	= GPIO_Pin_8;
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
* @brief  Initialize the HW platform
* @return   none
*/
void hw_platform_init(void)
{
	platform_misc_port_init();
	ADC_Module_Init();
	current_led_state = 0;
}


/**
* @brief	���ؼ�⵽�ĵ�ѹ�ļ���
* @param[in]  none
* @param[out] none
* @return     0:��ص�����		1�����е�
* @note     �ݶ�2�������ⶨ�ĵ�ѹ����3.15Vʱ����ʾϵͳ�ĵ�ѹ����
*			��صĹ�����ѹ��Χ��: 3.0 -- 4.0
*/

#define  LOW_POWER_TH	1960		//	
unsigned int hw_platform_get_PowerClass(void)
{ 
#if 1
	unsigned int  i,result = 0;
	unsigned short  temp[20];
	unsigned short	min,max;

	for(i = 0;i < 20;i++)
	{
		temp[i] = ADC_GetConversionValue(ADC1);		//�õ�ADת���Ľ��
		result += temp[i];
		if (i == 0)
		{
			min = temp[i];
			max = temp[i];
		}

		if (temp[i] < min)
		{
			min = temp[i];
		}

		if (temp[i] > max)
		{
			max = temp[i];
		}
	}

	//ȡ20��ֵ֮��,ȥ����Сֵ������ܣ���ȡƽ��ֵ���򵥵�ƽ���˲�
	result -= min;
	result -= max;
	result /= 18; 

	if (result > LOW_POWER_TH) 
	{
		return 1;
	}
	else
		return 0;
#endif

	//return 0;
}

/**
* @brief	�����״̬
* @param[in]  none
* @param[out] none
* @return     1:  ������    0: ���ڳ��
* @note  ���ڳ��ʱ�������IO������壬����������͵�ƽ 
*        ��Ҫע����ǣ��˺���ֻ���ڵ�֪����Դ�Ѿ������ǰ���£���ȥ���������塣��Ϊû�в������Դʱ�����IOҲ�ǵ͵�ƽ�ġ�
*		����IO�ı����ж�����ɳ��״̬�ļ�⣬���һֱ�ڽ��жϣ���˵�����ڳ�磬�����ʾ������
*/
unsigned int  hw_platform_ChargeState_Detect(void)
{
	if (charge_detect_io_cnt != last_charge_detect_io_cnt)
	{
		charge_state_cnt = 50000;
		last_charge_detect_io_cnt = charge_detect_io_cnt;
		return 0;
	}
	else
	{
		if (charge_state_cnt--)
		{
			return 0;
		}
	}
	return 1;
}

/**
* @brief	���USB�Ƿ����
* @param[in]  none
* @param[out] none
* @return     1:  ����    0: û�в���
* @note  �����Ҫ���ж�����ȥʵ�֣���ô����Ҫ�ڳ�ʼ��ʱ���˼��IO�����ⲿ�ж�
*		 ��������񼶲�ѯ����ô����֮�ʵ��ô˺���������Ƿ�������Դ                 
*/
unsigned int hw_platform_USBcable_Insert_Detect(void)
{
	unsigned int i;
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1))
	{
		for (i=0;i < 2000;i++);		//��ʱһС��ʱ�䣬��ֹ����Ϊ������ɵ�
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
		return 0;
}

/**
* @brief	LED���ƽӿ�
* @param[in]  unsigned int led	 LED_RED or  LED_GREEN  or LED_YELLOW
* @param[in]  unsigned int ctrl  0: close   1:open
* @return   none                 
*/
void hw_platform_led_ctrl(unsigned int led,unsigned int ctrl)
{
	if (led == LED_RED)
	{
		if (ctrl)
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_4);
		}
		else
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_4);
		}
	}
	else if (led == LED_GREEN)
	{
		if (ctrl)
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_5);
		}
		else
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_5);
		}
	}
	else if (led == LED_YELLOW)
	{
		if (ctrl)
		{
			GPIO_ResetBits(GPIOA, GPIO_Pin_6);
		}
		else
		{
			GPIO_SetBits(GPIOA, GPIO_Pin_6);
		}
	}
	else if (led == LED_BLUE)
	{
		if (ctrl)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_7);
		}
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_7);
		}
	}
}


/**
* @brief	Motor���ƽӿ�
* @param[in]  unsigned int delay  ��ʱһ��ʱ��,��λms
* @return   none                 
*/
void hw_platform_motor_ctrl(unsigned short delay)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
	if (delay < 5)
	{
		delay_ms(delay);
	}
	else
	{
		OSTimeDlyHMSM(0,0,0,delay);
	}
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
}

/**
* @brief	Trig���ƽӿ�
* @param[in]  unsigned int delay  ��ʱһ��ʱ��,��λms
* @return   none                 
*/
void hw_platform_trig_ctrl(unsigned short delay)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
	if (delay < 5)
	{
		delay_ms(delay);
	}
	else
	{
		OSTimeDlyHMSM(0,0,0,delay);
	}
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

/**
* @brief	�������򵥵Ŀ��ƽӿ�
* @param[in]  unsigned int delay	��ʱһ��ʱ��,��λms,����ʱ��
* @param[in]  unsigned int	beep_freq	�������ķ���Ƶ�ʣ�����
* @return   none 
*/
void hw_platform_beep_ctrl(unsigned short delay,unsigned int beep_freq)
{
	int i;
	for (i = 0; i < (delay*beep_freq)/2000;i++)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		delay_us(1000000/beep_freq);
		//delay_us(100);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		delay_us(1000000/beep_freq);
		//delay_us(400);
	}
}

/**
* @brief	��������Motorͬʱ���ƽӿ�
* @param[in]  unsigned int delay	��ʱһ��ʱ��,��λms,����ʱ��
* @param[in]  unsigned int	beep_freq	�������ķ���Ƶ�ʣ�����
* @return   none 
*/
void hw_platform_beep_motor_ctrl(unsigned short delay,unsigned int beep_freq)
{
	int i;
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
	for (i = 0; i < (delay*beep_freq)/2000;i++)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		delay_us(1000000/beep_freq);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		delay_us(1000000/beep_freq);
	}
	GPIO_ResetBits(GPIOA, GPIO_Pin_7);
}



/**
 * @brief ����ʱ��ģ���ṩ�Ķ�ʱ���ӿ�ʵ��LED��˸
*/
static void led_red_blink_timer_hook(void)
{
	current_led_state ^= LED_RED_MASK;
	hw_platform_led_ctrl(LED_RED,current_led_state & LED_RED_MASK);
}

static void led_blue_blink_timer_hook(void)
{
	current_led_state ^= LED_BLUE_MASK;
	hw_platform_led_ctrl(LED_BLUE,current_led_state & LED_BLUE_MASK);
}

static void led_green_blink_timer_hook(void)
{
	current_led_state ^= LED_GREEN_MASK;
	hw_platform_led_ctrl(LED_GREEN,current_led_state & LED_GREEN_MASK);
}

static void led_yellow_blink_timer_hook(void)
{
	current_led_state ^= LED_YELLOW_MASK;
	hw_platform_led_ctrl(LED_YELLOW,current_led_state & LED_YELLOW_MASK);
}

/**
 * @brief ��ʼLED��˸ָʾ
 * @param[in] unsigned int		led		
 * @param[in] unsigned short	delay	��˸��ʱ������Ҳ������˸Ƶ��,��λ10ms
 * @note ע��˽ӿڿ����ü���LEDͬʱ���ո��Բ�ͬ��Ƶ����˸
*/
void hw_platform_start_led_blink(unsigned int led,unsigned short delay)
{
	int ret;
	hw_platform_led_ctrl(led,1);
	if (led == LED_RED)
	{
		if (current_led_state & LED_RED_ON_MASK)
		{
			current_led_state |= LED_RED_MASK;
			ret = reset_timer(led_timer_h[0],V_TIMER_MODE_PERIODIC,delay*10,led_red_blink_timer_hook);
			assert(ret == 0);
		}
		else
		{
			current_led_state |= (LED_RED_MASK | LED_RED_ON_MASK);
			led_timer_h[0] = start_timer(V_TIMER_MODE_PERIODIC,delay*10,led_red_blink_timer_hook);
			assert(led_timer_h[0] != 0);
		}
	}
	else if (led == LED_BLUE)
	{
		if (current_led_state & LED_BLUE_ON_MASK)
		{
			current_led_state |= LED_BLUE_MASK;
			ret = reset_timer(led_timer_h[1],V_TIMER_MODE_PERIODIC,delay*10,led_blue_blink_timer_hook);
			assert(ret == 0);
		}
		else
		{
			current_led_state |= (LED_BLUE_MASK | LED_BLUE_ON_MASK);
			led_timer_h[1] = start_timer(V_TIMER_MODE_PERIODIC,delay*10,led_blue_blink_timer_hook);
			assert(led_timer_h[1] != 0);
		}
	}
	else if (led == LED_GREEN)
	{
		if (current_led_state & LED_GREEN_ON_MASK)
		{
			current_led_state |= LED_GREEN_MASK;
			ret = reset_timer(led_timer_h[2],V_TIMER_MODE_PERIODIC,delay*10,led_green_blink_timer_hook);
			assert(ret == 0);
		}
		else
		{
			current_led_state |= (LED_GREEN_MASK | LED_GREEN_ON_MASK);
			led_timer_h[2] = start_timer(V_TIMER_MODE_PERIODIC,delay*10,led_green_blink_timer_hook);
			assert(led_timer_h[2] != 0);
		}
	}
	else
	{
		if (current_led_state & LED_YELLOW_ON_MASK)
		{
			current_led_state |= LED_YELLOW_MASK;
			ret = reset_timer(led_timer_h[3],V_TIMER_MODE_PERIODIC,delay*10,led_yellow_blink_timer_hook);
			assert(ret == 0);
		}
		else
		{
			current_led_state |= (LED_YELLOW_MASK | LED_YELLOW_ON_MASK);
			led_timer_h[3] = start_timer(V_TIMER_MODE_PERIODIC,delay*10,led_yellow_blink_timer_hook);
			assert(led_timer_h[3] != 0);
		}
	}
}

/**
 * @brief �ر�ĳһ������˸��LED
*/
void hw_platform_stop_led_blink(unsigned int led)
{
	int ret;
	if (led == LED_RED)
	{
		hw_platform_led_ctrl(LED_RED,0);
		current_led_state &= ~LED_RED_MASK;
		current_led_state &= ~LED_RED_ON_MASK;
		ret = stop_timer(led_timer_h[0]);
		assert(ret == 0);
	}
	else if (led == LED_BLUE)
	{
		hw_platform_led_ctrl(LED_BLUE,0);
		current_led_state &= ~LED_BLUE_MASK;
		current_led_state &= ~LED_BLUE_ON_MASK;
		ret = stop_timer(led_timer_h[1]);
		assert(ret == 0);
	}
	else if (led == LED_GREEN)
	{
		hw_platform_led_ctrl(LED_GREEN,0);
		current_led_state &= ~LED_GREEN_MASK;
		current_led_state &= ~LED_GREEN_ON_MASK;
		ret = stop_timer(led_timer_h[2]);
		assert(ret == 0);
	}
	else
	{
		hw_platform_led_ctrl(LED_YELLOW,0);
		current_led_state &= ~LED_YELLOW_MASK;
		current_led_state &= ~LED_YELLOW_ON_MASK;
		ret = stop_timer(led_timer_h[3]);
		assert(ret == 0);
	}
}

/**
 * @brief ����IO
*/
void hw_platform_trip_io(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}


//����LED����˸�ӿ�
void hw_platform_led_blink_test(void)
{
	hw_platform_start_led_blink(LED_RED,6);
	OSTimeDlyHMSM(0,0,10,0);
	hw_platform_start_led_blink(LED_BLUE,20);
	OSTimeDlyHMSM(0,0,10,0);
	hw_platform_start_led_blink(LED_GREEN,50);
	OSTimeDlyHMSM(0,0,10,0);
	hw_platform_start_led_blink(LED_YELLOW,300);

	OSTimeDlyHMSM(0,0,10,0);

	hw_platform_stop_led_blink(LED_RED);

	OSTimeDlyHMSM(0,0,10,0);

	hw_platform_stop_led_blink(LED_BLUE);

	OSTimeDlyHMSM(0,0,10,0);

	hw_platform_stop_led_blink(LED_GREEN);

	OSTimeDlyHMSM(0,0,10,0);

	StartDelay(5000);

	while (DelayIsEnd()==0);

	hw_platform_stop_led_blink(LED_YELLOW);

}