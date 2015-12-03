/**
 * @file  Timebase.c
 * @brief use TIM2 to generate a time base 
 * @version 1.0
 * @author kent.zhou
 * @date 2015��10��10��
 * @note
*/
#include "stm32f10x_lib.h" 

#include "TimeBase.h"
#include "JMemory.h"
#include <assert.h>


static TimerIsrHook	timer_isr_hook;

/**
 * @brief     us ����ʱ
 * @param[in] unsigned int time ��ʱ����
*/
void delay_us(unsigned int time)
{    
	unsigned int i=0;  
	while(time--)
	{
		i=8;  
		while(i--) ;    
	}
}

/**
 * @brief     ms����ʱ
 * @param[in] unsigned int time ��ʱ����
*/
void delay_ms(unsigned int time)
{    
	unsigned int i=0;  
	while(time--)
	{
		i=10255; 
		while(i--) ;    
	}
}
/**
 * @brief     ��ʼ��������ʱʱ���ļ�����TIM2,�趨����������1ms��ʱ��
 * @param[in] none
 * @param[out] none
 * @return none
 * @note   �˳�ʼ�������е�����BSP_IntVectSet(BSP_INT_ID_TIM2, TIM2_UpdateISRHandler)���������������趨TIM2���жϴ�������
 *				 ����ֲ��ʱ����Ҫ���ݲ�ͬ�����������жϴ������ķ����������޸ġ�       
*/
void TimeBase_Init(void)
{
	TIM_TimeBaseInitTypeDef						TIM_TimeBaseStructure;
	TIM_OCInitTypeDef							TIM_OCInitStructure;
	NVIC_InitTypeDef							NVIC_InitStructure;

	//��ʼ���ṹ�����
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_OCStructInit(&TIM_OCInitStructure);

	/*������Ӧʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  


	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler			= 1;      //72M�ļ���Ƶ��
	TIM_TimeBaseStructure.TIM_CounterMode		= TIM_CounterMode_Up; //���ϼ���
	TIM_TimeBaseStructure.TIM_Period			= (72000/2-1);      
	TIM_TimeBaseStructure.TIM_ClockDivision		= 0x0;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Channel 1, 2, 3 and 4 Configuration in Timing mode */
	TIM_OCInitStructure.TIM_OCMode				= TIM_OCMode_Timing;
//   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
//   TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse				= 0x0;

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);

	/* Enable the TIM2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= TIM2_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	= 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//timer_isr_hook = (TimerIsrHook)0;
}

#if 0

/**
 * @brief ������ʱ��
 * @param[in] TimerIsrHook hook_func		��ʱ�жϻص�����
*/
void start_timer(TimerIsrHook hook_func)
{
	if (timer_isr_hook)
	{
		if (timer_isr_hook == hook_func)
		{
			return;
		}
		else
		{
			while(1);		//error
		}
	}
	timer_isr_hook = hook_func;
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM counter enable */
	TIM_Cmd(TIM2, ENABLE);
}


/**
 * @brief �رն�ʱ��
*/
void stop_timer(void)
{
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	/* TIM counter enable */
	TIM_Cmd(TIM2, DISABLE);
	timer_isr_hook = (TimerIsrHook)0;
}


/**
 * @brief TIM2������ж�ISR
 * @param[in] none
 * @return none
 * @note  TIM2���жϷ���������
*/
void TIM2_UpdateISRHandler(void)
{    
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		if (timer_isr_hook)
		{
			timer_isr_hook();
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}
#endif



typedef struct v_timer_t
{
	unsigned char	mode;
	unsigned int	count;
	unsigned int	dly;
	TimerIsrHook	timer_hook;
	struct v_timer_t	*prev;
	struct v_timer_t	*next;
}V_TIMER;

struct 
{
	V_TIMER		*v_timer_tail;
	unsigned int	v_timer_cnt;
}v_timer_list;

/**
 * @brief ������ʱ��
 * @param[in] unsigned char mode		V_TIMER_MODE_ONE_SHOT		or      V_TIMER_MODE_PERIODIC
 * @param[in] unsigned int dly			��V_TIMER_MODE_ONE_SHOTģʽ�£���ʾ��ʱʱ��
 *										��V_TIMER_MODE_PERIODICģʽ�£���ʾ��ʱ����
 * @param[in] TimerIsrHook hook_func	��ʾ��ʱһ����Ҫִ�еĻص�����
 * @return 0��  �ѿռ䲻��
 *         else  �������ⶨʱ���ľ��
 * @note Ӧ���ڵ��ô˽ӿ�ʱ��֪���Ѿ��ж��ٸ����ⶨʱ������Ӳ����ʱ���£����Է���ֵ�൱�ڶ�ʱ�����������
 *       ���ڴ����ⶨʱ���ǻ���Ӳ����ʱ��TIME2ʵ�ֵģ�TIMER2��ʼ��Ϊ1ms�жϣ����Զ�ʱ����С��λΪ1ms
*/
VTIMER_HANDLE start_timer(unsigned char mode,unsigned int dly, TimerIsrHook hook_func)
{
	V_TIMER	*p_vtimer;
	p_vtimer = (V_TIMER*)Jmalloc(sizeof(V_TIMER));
	if (p_vtimer == 0)
	{
		return 0;
	}
	p_vtimer->mode = mode;
	p_vtimer->dly = dly;
	p_vtimer->timer_hook = hook_func;
	p_vtimer->prev = 0;
	p_vtimer->next = 0;
	p_vtimer->count = 0;

	if (v_timer_list.v_timer_cnt == 0)
	{
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
		/* TIM counter enable */
		TIM_Cmd(TIM2, ENABLE);
	}
	else
	{
		p_vtimer->prev = v_timer_list.v_timer_tail;
		v_timer_list.v_timer_tail->next = p_vtimer;
	}
	v_timer_list.v_timer_tail = p_vtimer;
	v_timer_list.v_timer_cnt ++;

	return (VTIMER_HANDLE)p_vtimer;
}

/**
 * @brief �ж����ⶨʱ���ľ���Ƿ�Ϸ��������������ⶨʱ�������ʵ����ÿ����ʱ�����ƿ��ָ�룩�Ƿ��ڶ�ʱ��������
 * @param[in]  VTIMER_HANDLE v_timer_h
*/
static int check_vtimer_handle(VTIMER_HANDLE v_timer_h)
{
	V_TIMER	*p_vtimer;

	p_vtimer = v_timer_list.v_timer_tail;
	while (p_vtimer)
	{
		if (p_vtimer == (V_TIMER*)v_timer_h)
		{
			return 1;
		}
		p_vtimer = p_vtimer->prev;
	}

	return 0;
}

/**
 * @brief ����ĳ��ʱ��
 * @param[in]  VTIMER_HANDLE v_timer_h
*/
int reset_timer(VTIMER_HANDLE v_timer_h,unsigned char mode,unsigned int dly, TimerIsrHook hook_func)
{
	if (check_vtimer_handle(v_timer_h) == 0)
	{
		return -1;
	}

	V_TIMER	*p_vtimer = (V_TIMER*)v_timer_h;

	p_vtimer->count = 0;
	p_vtimer->mode = mode;
	p_vtimer->dly = dly;
	p_vtimer->timer_hook = hook_func;

	return 0;
}

/**
 * @brief �ر�ĳ��ʱ��
 * @param[in]  VTIMER_HANDLE v_timer_h
*/
int stop_timer(VTIMER_HANDLE v_timer_h)
{
	if (check_vtimer_handle(v_timer_h) == 0)
	{
		return 0;
	}

	V_TIMER	*p_vtimer = (V_TIMER*)v_timer_h;

	if (p_vtimer->next)
	{
		p_vtimer->next->prev = p_vtimer->prev;
	}
	else
	{
		v_timer_list.v_timer_tail = p_vtimer->prev;
	}
	
	if(p_vtimer->prev)
	{
		p_vtimer->prev->next = p_vtimer->next;
	}

	v_timer_list.v_timer_cnt--;
	Jfree(p_vtimer);
	
	if (v_timer_list.v_timer_cnt == 0)
	{
		TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
		/* TIM counter enable */
		TIM_Cmd(TIM2, DISABLE);
	}

	return 0;
}


/**
 * @brief ǿ�ƿ���Ӳ��ʱ��
*/
void start_real_timer(void)
{
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	/* TIM counter enable */
	TIM_Cmd(TIM2, ENABLE);
	return;
}

/**
 * @brief ǿ�ƹر�Ӳ��ʱ��
*/
void stop_real_timer(void)
{
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
	/* TIM counter enable */
	TIM_Cmd(TIM2, DISABLE);
	return;
}

/**
 * @brief TIM2������ж�ISR
 * @param[in] none
 * @return none
 * @note  TIM2���жϷ���������
*/
void TIM2_UpdateISRHandler(void)
{    
	V_TIMER *p_vtimer;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		p_vtimer = v_timer_list.v_timer_tail;
		while (p_vtimer)
		{
			p_vtimer->count++;
			if (p_vtimer->count == p_vtimer->dly)
			{
				if (p_vtimer->mode == V_TIMER_MODE_ONE_SHOT)
				{
					if (p_vtimer->next)
					{
						p_vtimer->next->prev = p_vtimer->prev;
					}
					else
					{
						v_timer_list.v_timer_tail = p_vtimer->prev;
					}

					if(p_vtimer->prev)
					{
						p_vtimer->prev->next = p_vtimer->next;
					}

					v_timer_list.v_timer_cnt--;
					Jfree(p_vtimer);

					if (v_timer_list.v_timer_cnt == 0)
					{
						TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
						/* TIM counter enable */
						TIM_Cmd(TIM2, DISABLE);
					}
				}
				else
				{
					p_vtimer->count = 0;
				}

				if (p_vtimer->timer_hook)
				{
					p_vtimer->timer_hook();
				}
			}
			p_vtimer = p_vtimer->prev;
		}
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}


//===============================================����timer�ṩ�Ľӿ�ʵ����ʱ����================================
static int Delay_end_flag;
//����timerʵ����ʱ����ʱ���жϹ��Ӻ���
void delay_func_hook(void)
{
	Delay_end_flag = 1;
}

/**
 * @brief ��ʼ����TIM2��ʱ����
 * @param[in] unsigned int nTime   ��λms
 * @return  none
*/
void StartDelay(unsigned short nTime)
{
	int	ret;
    Delay_end_flag = 0;
	ret = start_timer(V_TIMER_MODE_ONE_SHOT,nTime,delay_func_hook);
	assert(ret != 0);
}


/**
 * @brief �ж���ʱʱ���Ƿ�
 * @param[in] none
 * @return 1: ��ʱ��
 *        0: ��ʱδ��
*/
unsigned char DelayIsEnd(void)
{
	return	Delay_end_flag; 
}
