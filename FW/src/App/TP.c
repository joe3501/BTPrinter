#include "Type.h"
#include "stm32f10x_lib.h" 
#include "TP.h"
#include "basic_fun.h"
#include "print_head.h"
#include "Esc_p.h"
#include "uart.h"
#include "Terminal_Para.h"
#include "DotFill.h"
#include "PaperDetect.h"
#include "Event.h"
#include "ThermalDetect.h"
#include <string.h>
#include <stdio.h>

#ifdef DEBUG_VER
extern unsigned short debug_buffer[];
extern unsigned int debug_cnt;
#endif

extern   void NVIC_DisableIRQ(unsigned char	irq_channel);
extern   void NVIC_EnableIRQ(unsigned char	irq_channel);
#define Half_Step
enum
{
    TP_CMD_PRINT_DOTLINE,
    TP_CMD_FEED_TO_MARK,
    TP_CMD_FEED,
    TP_CMD_MAX
};

//==================================================================
#define TIMER1_MS_TO_CNT(ms)	((uint16_t)(1000*(ms)-1))
#define TIMER1_CNT_TO_MS(cnt)	(cnt+1)
#define TP_MINWAIT_TIME	(TIMER1_MS_TO_CNT(0.1000))//100
#define TIMER1_SPEED_TO_CNT(mm)	(((uint16_t)((SYSPCLK/ PR_Val)/ (mm*16))) - TP_MINWAIT_TIME)
//==================================================================

// 首先确定每个点行需要分几次加热(TP_MAX_HEAT_STROBE)，然后确定每次加热需要走纸的步数(TP_MAX_STROBE_STEP)
// 最终必须保证(TP_MAX_HEAT_STROBE*TP_MAX_STROBE_STEP) = 一个点行的总步进数
#ifdef Half_Step
#define TP_MAX_HEAT_STROBE	(1) // 分几次加热，必须保证(LineDot/8/TP_MAX_HEAT_STROBE)是整数
#define TP_MAX_STROBE_STEP	(4) // 每个加热Strobe上的步进
#else
#define TP_MAX_HEAT_STROBE	(1)
#define TP_MAX_STROBE_STEP	(2) // 每个加热Strobe上的步进
#endif


#define TP_MAX_HEAT_DOT		(64)		// 每次最多能够加热的点数，必须大等于8
//#define TP_MAX_HEAT_DOT		(48)		// 每次最多能够加热的点数，必须大等于8

#define TpMinWaitTime	(TIMER1_MS_TO_CNT(0.100))


#define T1_PCLK_DIV     3


#define DISABLE_TIMER_INTERRUPT()	do{	\
		NVIC_DisableIRQ(TIM3_IRQChannel); \
	}while(0)

#define ENABLE_TIMER_INTERRUPT()	do{	\
	NVIC_EnableIRQ(TIM3_IRQChannel); \
	}while(0)

//GPIO_SetBits(GPIOB,GPIO_Pin_0);
#define LATCH_HIGH()	do{	\
		GPIOB->BSRR = GPIO_Pin_0;	\
	}while(0)

#define LATCH_LOW()	do{	\
	   GPIOB->BRR = GPIO_Pin_0;	\
	}while(0)


//====================================================================
//PB.2
#define MOTOR_PWR_ON()    do{ \
        GPIOB->BSRR = GPIO_Pin_2; \
        }while(0)

#define MOTOR_PWR_OFF()   do{ \
		GPIOB->BRR = GPIO_Pin_2; \
        }while(0)

//PE.7
#define MOTOR_PHASE_1A_HIGH()  do{ \
       GPIOE->BSRR = GPIO_Pin_7; \
       }while(0)

#define MOTOR_PHASE_1A_LOW()   do{ \
       GPIOE->BRR = GPIO_Pin_7; \
}while(0)

//PE.8
#define MOTOR_PHASE_1B_HIGH()  do{ \
       GPIOE->BSRR = GPIO_Pin_8; \
       }while(0)

#define MOTOR_PHASE_1B_LOW()   do{ \
       GPIOE->BRR = GPIO_Pin_8;  \
       }while(0)

//PE.10
#define MOTOR_PHASE_2A_HIGH()  do{ \
       GPIOE->BSRR = GPIO_Pin_10; \
       }while(0)

#define MOTOR_PHASE_2A_LOW()   do{ \
       GPIOE->BRR = GPIO_Pin_10; \
       }while(0)

//PE.9
#define MOTOR_PHASE_2B_HIGH()  do{ \
	GPIOE->BSRR = GPIO_Pin_9; \
	}while(0)

#define MOTOR_PHASE_2B_LOW()   do{  \
	GPIOE->BRR = GPIO_Pin_9; \
	}while(0)

//PC.4
#define STROBE_0_ON()     do{ \
	GPIOC->BSRR = GPIO_Pin_4; \
	}while(0)

#define STROBE_0_OFF()    do{ \
	GPIOC->BRR = GPIO_Pin_4; \
        }while(0)

//PC.5
#define STROBE_1_ON()     do{ \
	GPIOC->BSRR = GPIO_Pin_5; \
	}while(0)

#define STROBE_1_OFF()    do{ \
	GPIOC->BRR = GPIO_Pin_5; \
	}while(0)
//======================================================================

//PB.1
#define PRN_POWER_CHARGE() \
	do{ \
		GPIOB->BSRR = GPIO_Pin_1; \
	}while(0)

#define PRN_POWER_DISCHARGE() \
	do{ \
	   GPIOB->BRR = GPIO_Pin_1;\
	}while(0)

//======================================================================

typedef enum
{
    TPSTATE_IDLE = 0,
    TPSTATE_PRE_IDLE,
    TPSTATE_START,
    TPSTATE_HEAT_WITH_FEED,       // 开始加热，走纸，时间最长为马达步进时间(下一状态为TPSTATE_HEAT1_FEED_B)，最短为加热时间(下一状态为TPSTATE_HEAT1_STOP_A)
    TPSTATE_HEAT_WITHOUT_FEED,
    TPSTATE_FEED,
    TPSTATE_FEED_TO_MARK,
    TPSTATE_FEED_FIND_MARK,
    TPSTATE_FEED_AFTERMARK,
    TPSTATE_CUT_PRE_FEED,		// 进刀到Home位置
    TPSTATE_CUT_FEED,			// 进刀到希望的位置(Partial/Full)
    TPSTATE_CUT_REVERSE,		// 先退刀到Home位置
    TPSTATE_CUT_REVERSE1,		// 再退一段距离
    TPSTATE_WAIT_TIME,          //唤醒等待时间
    TPSTATE_MAX
} TPSTATE_T;

typedef struct
{
	TPSTATE_T state;
	uint8_t phase;
	uint8_t cutphase;
	uint8_t cutmode;        //切刀模式:0:全切 1:半切
	uint16_t repeat;

	uint32_t heat;
	uint32_t heat_setting;
	uint32_t heat_remain;
	uint32_t feed_time[TP_MAX_STROBE_STEP];

	uint8_t feed_step;		// 单个Strobe加热时马达步进计数器
	uint8_t strobe_step;	// 单个点行加热时需要分的Strobe数的计数器
	uint8_t accel;
	uint16_t head;
	volatile uint16_t tail;
	uint16_t feedmax;
	uint8_t pause;
	uint8_t heat_max_cnt;
	uint8_t heat_cnt;
	uint8_t heat_buf[((LineDot/TP_MAX_HEAT_STROBE)+(TP_MAX_HEAT_DOT-7)-1)/(TP_MAX_HEAT_DOT-7)][LineDot/8];
	int32_t markbefore;
} TP_T;

static TP_T tp;
static uint8_t TP_dot[16][LineDot/8+1];		// 增加一个控制位


extern uint8_t clr_all_dot=0;

/**
* @brief	打印头控制IO及定时器的初始化
* @note                  
*/
void TPInit(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef						TIM_TimeBaseStructure;
	NVIC_InitTypeDef							NVIC_InitStructure;

	//PRN_STROBE0 -- PC.4   PRN_STROBE1 -- PC.5
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//MOT_PHASE1A -- PE.7   MOT_PHASE1B -- PE.8   MOT_PHASE2B -- PE.9	MOT_PHASE2A -- PE.10  
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	//PRN_LATCH	-- PB.0	 PRN_POWER -- PB.1  MOT_POWER -- PB.2
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//MOT_STATUS -- PE.11
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/*开启相应时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  

	//((1+TIM_Prescaler )/72M)*(1+TIM_Period ) = 1ms定时
	/* Time Base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler			= 71;      //1us的计数频率
	TIM_TimeBaseStructure.TIM_CounterMode		= TIM_CounterMode_Up; //向上计数
	TIM_TimeBaseStructure.TIM_Period			= (1000-1);      
	TIM_TimeBaseStructure.TIM_ClockDivision		= 0x0;

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* set the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= TIM3_IRQChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		//中断优先级最高
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
	TIM_ARRPreloadConfig(TIM3,ENABLE);
	TIM_Cmd(TIM3,ENABLE);


	//初始化控制时序
	MOTOR_PWR_OFF();
	STROBE_0_OFF();
	STROBE_1_OFF();

	MOTOR_PHASE_1A_LOW();
	MOTOR_PHASE_1B_LOW();
	MOTOR_PHASE_2A_LOW();
	MOTOR_PHASE_2B_LOW();
	//===========================

	tp.head = 0;
	tp.tail = 0;
	tp.state = TPSTATE_IDLE;
	tp.pause = 0;

}


extern void WakeUpTP_MODE1(void)
{
    volatile uint16_t len;

    PrintBufToZero();
    esc_sts[current_channel].bitmap_flag = 0;
	memset(esc_sts[current_channel].dot, 0, sizeof(esc_sts[current_channel].dot));
	esc_sts[current_channel].start_dot = 0;
    max_start_col =0;
	esc_sts[current_channel].dot_minrow = ARRAY_SIZE(esc_sts[current_channel].dot[0]);

    len =  LineDot/8;
    while (len--)
    {
         print_head_spi_send_byte(0);
    }
    LATCH_LOW();
    len = 100;
    while (len--);
    LATCH_HIGH();
    tp.tail=tp.head;
    TPIntSetIdle();
    clr_all_dot=1;

}


static void TPForwardStep(int direction)
{
	//trip4();
    PRN_POWER_DISCHARGE();
#if defined(PT486) || defined(PT487)||defined(PT48D)||defined(PT1043)|| defined(PT48G)
#ifdef Half_Step
	switch (tp.phase & 0x07)
	{

        case 0:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;
        case 1:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_LOW();
        break;
        case 2:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 3:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 4:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 5:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_LOW();
        break;
        case 6:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;
        case 7:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;

     }

#else
        switch (tp.phase & 0x03)
        {

		   case 0:
                MOTOR_PHASE_1A_HIGH();
                MOTOR_PHASE_1B_LOW();
                MOTOR_PHASE_2A_HIGH();
                MOTOR_PHASE_2B_LOW();
            break;
            case 1:
                MOTOR_PHASE_1A_HIGH();
                MOTOR_PHASE_1B_LOW();
                MOTOR_PHASE_2A_LOW();
                MOTOR_PHASE_2B_HIGH();
            break;
            case 2:
                MOTOR_PHASE_1A_LOW();
                MOTOR_PHASE_1B_HIGH();
                MOTOR_PHASE_2A_LOW();
                MOTOR_PHASE_2B_HIGH();
            break;
            case 3:
                MOTOR_PHASE_1A_LOW();
                MOTOR_PHASE_1B_HIGH();
                MOTOR_PHASE_2A_HIGH();
                MOTOR_PHASE_2B_LOW();
            break;

         }
#endif

#else
#ifdef Half_Step
	switch (tp.phase & 0x07)
	{
        case 7:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;
        case 6:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_LOW();
        break;
        case 5:
            MOTOR_PHASE_1A_HIGH();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 4:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 3:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_HIGH();
        break;
        case 2:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_LOW();
            MOTOR_PHASE_2B_LOW();
        break;
        case 1:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_HIGH();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;
        case 0:
            MOTOR_PHASE_1A_LOW();
            MOTOR_PHASE_1B_LOW();
            MOTOR_PHASE_2A_HIGH();
            MOTOR_PHASE_2B_LOW();
        break;
     }

#else
        switch (tp.phase & 0x03)
        {
            case 3:
                MOTOR_PHASE_1A_HIGH();
                MOTOR_PHASE_1B_LOW();
                MOTOR_PHASE_2A_HIGH();
                MOTOR_PHASE_2B_LOW();
            break;
            case 2:
                MOTOR_PHASE_1A_HIGH();
                MOTOR_PHASE_1B_LOW();
                MOTOR_PHASE_2A_LOW();
                MOTOR_PHASE_2B_HIGH();
            break;
            case 1:
                MOTOR_PHASE_1A_LOW();
                MOTOR_PHASE_1B_HIGH();
                MOTOR_PHASE_2A_LOW();
                MOTOR_PHASE_2B_HIGH();
            break;
            case 0:
                MOTOR_PHASE_1A_LOW();
                MOTOR_PHASE_1B_HIGH();
                MOTOR_PHASE_2A_HIGH();
                MOTOR_PHASE_2B_LOW();
            break;
         }
#endif
#endif
     if(TPPrinterMark() == FALSE)        // find mark
     {
         tp.markbefore = 0;
     }
     else if(tp.markbefore >= 0)
     {
         tp.markbefore++;
     }
     PRN_POWER_CHARGE();
	tp.phase += direction;
}

static uint16_t const TpAccelerationSteps[] =
{
	TIMER1_MS_TO_CNT(5.579),
	TIMER1_MS_TO_CNT(4.308),
	TIMER1_MS_TO_CNT(2.614),
	TIMER1_MS_TO_CNT(2.500),
	TIMER1_MS_TO_CNT(2.426),
	TIMER1_MS_TO_CNT(2.272),
	TIMER1_MS_TO_CNT(2.144),
	TIMER1_MS_TO_CNT(2.035),
	TIMER1_MS_TO_CNT(1.941),
	TIMER1_MS_TO_CNT(1.859),
	TIMER1_MS_TO_CNT(1.786),
	TIMER1_MS_TO_CNT(1.721),
	TIMER1_MS_TO_CNT(1.663),
	TIMER1_MS_TO_CNT(1.610),
	TIMER1_MS_TO_CNT(1.561),
	TIMER1_MS_TO_CNT(1.517),
	TIMER1_MS_TO_CNT(1.477),
	TIMER1_MS_TO_CNT(1.439),
	TIMER1_MS_TO_CNT(1.404),
	TIMER1_MS_TO_CNT(1.372),
	TIMER1_MS_TO_CNT(1.342),
	TIMER1_MS_TO_CNT(1.313),
	TIMER1_MS_TO_CNT(1.287),
	TIMER1_MS_TO_CNT(1.261),
	TIMER1_MS_TO_CNT(1.238),
	TIMER1_MS_TO_CNT(1.215),
	TIMER1_MS_TO_CNT(1.194),
	TIMER1_MS_TO_CNT(1.174),
	TIMER1_MS_TO_CNT(1.155),
	TIMER1_MS_TO_CNT(1.136),
	TIMER1_MS_TO_CNT(1.119),
	TIMER1_MS_TO_CNT(1.102),
	TIMER1_MS_TO_CNT(1.086),
	TIMER1_MS_TO_CNT(1.071),
	TIMER1_MS_TO_CNT(1.056),
	TIMER1_MS_TO_CNT(1.042),
	TIMER1_MS_TO_CNT(1.029),
	TIMER1_MS_TO_CNT(1.016),
	TIMER1_MS_TO_CNT(1.003),
	TIMER1_MS_TO_CNT(0.991),
	TIMER1_MS_TO_CNT(0.979),
	TIMER1_MS_TO_CNT(0.968),
	TIMER1_MS_TO_CNT(0.957),
	TIMER1_MS_TO_CNT(0.947),
	TIMER1_MS_TO_CNT(0.936),
	TIMER1_MS_TO_CNT(0.927),
	TIMER1_MS_TO_CNT(0.917),
	TIMER1_MS_TO_CNT(0.908),
	TIMER1_MS_TO_CNT(0.899),
	TIMER1_MS_TO_CNT(0.890),
	TIMER1_MS_TO_CNT(0.882),
	TIMER1_MS_TO_CNT(0.873),
	TIMER1_MS_TO_CNT(0.865),
	TIMER1_MS_TO_CNT(0.857),
	TIMER1_MS_TO_CNT(0.850),
	TIMER1_MS_TO_CNT(0.842),
	TIMER1_MS_TO_CNT(0.835),
	TIMER1_MS_TO_CNT(0.828),
	TIMER1_MS_TO_CNT(0.821),
	TIMER1_MS_TO_CNT(0.815),
	TIMER1_MS_TO_CNT(0.808),
	TIMER1_MS_TO_CNT(0.802),
	TIMER1_MS_TO_CNT(0.796),
	TIMER1_MS_TO_CNT(0.789),
	TIMER1_MS_TO_CNT(0.784),
	TIMER1_MS_TO_CNT(0.778),
	TIMER1_MS_TO_CNT(0.772),
	TIMER1_MS_TO_CNT(0.766),
	TIMER1_MS_TO_CNT(0.761),
	TIMER1_MS_TO_CNT(0.756),
	TIMER1_MS_TO_CNT(0.750),
	TIMER1_MS_TO_CNT(0.745),
	TIMER1_MS_TO_CNT(0.740),
	TIMER1_MS_TO_CNT(0.735),
	TIMER1_MS_TO_CNT(0.731),
	TIMER1_MS_TO_CNT(0.726),
	TIMER1_MS_TO_CNT(0.721),
	TIMER1_MS_TO_CNT(0.717),
	TIMER1_MS_TO_CNT(0.712),
	TIMER1_MS_TO_CNT(0.708),
	TIMER1_MS_TO_CNT(0.704),
	TIMER1_MS_TO_CNT(0.699),
	TIMER1_MS_TO_CNT(0.695),
	TIMER1_MS_TO_CNT(0.691),
	TIMER1_MS_TO_CNT(0.687),
	TIMER1_MS_TO_CNT(0.683),
	TIMER1_MS_TO_CNT(0.679),
	TIMER1_MS_TO_CNT(0.675),
	TIMER1_MS_TO_CNT(0.672),
	TIMER1_MS_TO_CNT(0.668),
	TIMER1_MS_TO_CNT(0.664),
	TIMER1_MS_TO_CNT(0.661),
	TIMER1_MS_TO_CNT(0.657),
	TIMER1_MS_TO_CNT(0.654),
	TIMER1_MS_TO_CNT(0.651),
	TIMER1_MS_TO_CNT(0.647),
	TIMER1_MS_TO_CNT(0.644),
	TIMER1_MS_TO_CNT(0.641),
	TIMER1_MS_TO_CNT(0.637),
	TIMER1_MS_TO_CNT(0.634),
	TIMER1_MS_TO_CNT(0.631),
	TIMER1_MS_TO_CNT(0.628),
	TIMER1_MS_TO_CNT(0.625),
	TIMER1_MS_TO_CNT(0.622),
	TIMER1_MS_TO_CNT(0.619),
	TIMER1_MS_TO_CNT(0.616),
	TIMER1_MS_TO_CNT(0.614),
	TIMER1_MS_TO_CNT(0.611),
	TIMER1_MS_TO_CNT(0.608),
    TIMER1_MS_TO_CNT(0.605),
    TIMER1_MS_TO_CNT(0.603),
    TIMER1_MS_TO_CNT(0.600),
#if !defined(LOW_5V_PRINT)
    TIMER1_MS_TO_CNT(0.597),
    TIMER1_MS_TO_CNT(0.595),
    TIMER1_MS_TO_CNT(0.592),
    TIMER1_MS_TO_CNT(0.590),
    TIMER1_MS_TO_CNT(0.587),
    TIMER1_MS_TO_CNT(0.585),
    TIMER1_MS_TO_CNT(0.582),
    TIMER1_MS_TO_CNT(0.580),
    TIMER1_MS_TO_CNT(0.577),
    TIMER1_MS_TO_CNT(0.575),
    TIMER1_MS_TO_CNT(0.573),
    TIMER1_MS_TO_CNT(0.570),
    TIMER1_MS_TO_CNT(0.568),
//=========================
    TIMER1_MS_TO_CNT(0.558),
	TIMER1_MS_TO_CNT(0.548),
	TIMER1_MS_TO_CNT(0.538),
	TIMER1_MS_TO_CNT(0.529),
	TIMER1_MS_TO_CNT(0.520),
	TIMER1_MS_TO_CNT(0.512),
	TIMER1_MS_TO_CNT(0.504),
	TIMER1_MS_TO_CNT(0.497),
	TIMER1_MS_TO_CNT(0.489),
	TIMER1_MS_TO_CNT(0.482),
	TIMER1_MS_TO_CNT(0.476),
	TIMER1_MS_TO_CNT(0.469),
	TIMER1_MS_TO_CNT(0.463),
	TIMER1_MS_TO_CNT(0.457),
	TIMER1_MS_TO_CNT(0.452),
	TIMER1_MS_TO_CNT(0.446),
	TIMER1_MS_TO_CNT(0.441),
	TIMER1_MS_TO_CNT(0.436),
	TIMER1_MS_TO_CNT(0.431),
	TIMER1_MS_TO_CNT(0.426),
	TIMER1_MS_TO_CNT(0.422),
	TIMER1_MS_TO_CNT(0.417),
	TIMER1_MS_TO_CNT(0.413),
	TIMER1_MS_TO_CNT(0.409),
	TIMER1_MS_TO_CNT(0.405),
	TIMER1_MS_TO_CNT(0.401),
	#if defined(HIGH_8V_PRINT)
	TIMER1_MS_TO_CNT(0.397),
	TIMER1_MS_TO_CNT(0.393),
	TIMER1_MS_TO_CNT(0.390),
    TIMER1_MS_TO_CNT(0.386),
	TIMER1_MS_TO_CNT(0.383),
	TIMER1_MS_TO_CNT(0.379),
	TIMER1_MS_TO_CNT(0.376),
	TIMER1_MS_TO_CNT(0.373),
	TIMER1_MS_TO_CNT(0.370),
	TIMER1_MS_TO_CNT(0.367),
	TIMER1_MS_TO_CNT(0.364),
	TIMER1_MS_TO_CNT(0.361),
	TIMER1_MS_TO_CNT(0.358),
	TIMER1_MS_TO_CNT(0.355),
	TIMER1_MS_TO_CNT(0.353),
	TIMER1_MS_TO_CNT(0.350),
	#endif
#endif
};

// 加热时间根据电压调整
static uint32_t TPHeatVoltageAdj(uint32_t tm)
{

#if defined(TP_VOLTAGE_SNS)

#endif
   	return tm;


}
#if defined(TEMP_SNS_ENABLE)
// 加热时间根据热敏头的温度进行调整
static uint32_t TPHeatThermalAdj(uint32_t tm,int16_t temp)
{
	static uint8_t const Temperater_Ratio[]=
	{
   // 比例        温度
	250,248,242,238,230,228,224,220,216,210,//-20~-11
	202,194,186,178,172,168,164,160,156,152,//-10~-1
	148,144,140,136,132,128,124,122,120,118,//0-9
	116,114,116,112,111,110,109,108,107,106,//10-19
	105,104,103,102,101,100, 99, 98, 97, 96,//20-29
	 95, 94, 93, 92, 91, 90, 89, 88, 87, 86,//30-39
     85, 84, 83, 82, 81, 80, 79, 78, 77, 76,//40-49
     75, 74, 73, 72, 71, 70, 69, 68, 67, 66,//50-59
     65, 64, 63, 62, 61, 60, 59, 58, 57, 56,//60-69
     55, 54, 53, 52, 51, 50, 50, 50, 50, 50,//70-79
     50, 50, 50, 50, 50, 50,                //80-85
	};
    if
((temp >= -20)&&(temp <= 85))
    {
	   tm = tm *Temperater_Ratio[temp+20]/100;
    }
	else if (temp < -20)
	{
       tm = tm *Temperater_Ratio[0]/100;
	}
    else if (temp>85)
    {
       tm = tm /2;
    }

	return tm;
}
#endif

// 加热前几行的加热时间调整
static uint32_t TPHeatPreLineAdj(uint32_t tm)
{
//	TODO:
//	tm += ((uint32_t)TpAccelerationSteps[tp.accel]) * 10 / 100;
	return tm;
}

static uint32_t TPHeatDotsAdj(uint32_t tm,uint16_t dots)
{

	//TP_MAX_HEAT_DOT变化的话需做相应调整
	#if (TP_MAX_HEAT_DOT == 64)
	const uint8_t dot_ratio_tbl[TP_MAX_HEAT_DOT/4]=
	{
       70, 74, 78, 82, 86, 88,//0-24
	   90, 92, 93, 94, 95, 96,//24-48
	   97, 98,100,100,        //48-64
	};
	#elif (TP_MAX_HEAT_DOT == 48)
	const uint8_t dot_ratio_tbl[TP_MAX_HEAT_DOT/4]=
	{
		70, 78, 86, 88,//0-24
		90, 93, 95, 96,//24-48
		97, 98,100,100,        //48-64
	};
	#else
	#error("No define dot_ratio_tbl");
	#endif

    if(dots<TP_MAX_HEAT_DOT)
	{
	  tm = tm * dot_ratio_tbl[dots/4]/100;
	}
	else
	{
	  tm = tm * dot_ratio_tbl[TP_MAX_HEAT_DOT/4-1]/100;
	}

    return tm;
}
static void TPAdjustStepTime(uint8_t heat_cnt,uint16_t max_heat_dots)
{
	uint16_t heat;
	uint16_t time, time_sum;
	uint8_t i;

	heat = TPHeatVoltageAdj(tp.heat_setting);
	heat = TPHeatDotsAdj(heat,max_heat_dots);
	#if defined(TEMP_SNS_ENABLE)
	heat = TPHeatThermalAdj(heat,TPHTemperature());
	#endif
	heat = TPHeatPreLineAdj(heat);
	tp.heat = heat;

    heat *= heat_cnt;

    heat += TpMinWaitTime;

	while(1)
	{
		time_sum = 0;
		for(i=0; i<TP_MAX_STROBE_STEP; i++)
		{
			if((tp.accel+i) < ARRAY_SIZE(TpAccelerationSteps))
			{
				time = TpAccelerationSteps[tp.accel+i];
			}
			else
			{
				time = TpAccelerationSteps[ARRAY_SIZE(TpAccelerationSteps)-1];
			}
			tp.feed_time[i] = time;
			time_sum += time;
		}
		if(time_sum < heat)
		{
			if(tp.accel)
			{
				tp.accel--;
			}
			else
			{
				for(i=0; i<TP_MAX_STROBE_STEP; i++)
				{
					tp.feed_time[i] = heat/TP_MAX_STROBE_STEP;
				}
				break;
			}
		}
		else
		{
			if((tp.accel+TP_MAX_STROBE_STEP) < (ARRAY_SIZE(TpAccelerationSteps)-1))
			{
				tp.accel += TP_MAX_STROBE_STEP;
			}
			else
			{
				tp.accel = (ARRAY_SIZE(TpAccelerationSteps)-1);
			}
			break;
		}
	}
}
static uint16_t TPGetStepTime(void)
{
	uint16_t time;

	time = TpAccelerationSteps[tp.accel];
	if(tp.accel < (ARRAY_SIZE(TpAccelerationSteps)-1))
	{
		tp.accel++;
	}
	return time;
}

static void TPSetTimeCnt(uint16_t tm)
{
	TIM3->ARR = tm;
	TIM3->EGR = TIM_EventSource_Update;		//这个很重要，软件产生一次更新事件，才能将当前设置的溢出值更新到它的影子寄存器！！！（此问题折磨了2天之久）
	//printf("current tm=%d(us)\r\n",TIMER1_CNT_TO_MS(tm));		//for debug
}

static uint8_t TPFeedStep(void)
{
	TPSetTimeCnt(TPGetStepTime());	// set timer
	if(TPPrinterReady() != TRUE )
	{
        return 0;
	}
	if(tp.feedmax)
	{
		TPForwardStep(1);
		tp.feedmax--;
	}
	if(tp.feedmax)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


static void TPIntSetPreIdle(void)
{
	//trip4();
	STROBE_0_OFF(); 	// stop heat
	STROBE_1_OFF(); 	// stop heat
	tp.feedmax = 60*1;		// 每1ms中断一次
	tp.state = TPSTATE_PRE_IDLE;
}

static void TPIntSetIdle(void)
{
	//trip3();
	STROBE_0_OFF(); 	// stop heat
	STROBE_1_OFF(); 	// stop heat
	DISABLE_TIMER_INTERRUPT();				// disable interrupt
	MOTOR_PWR_OFF();
	tp.state = TPSTATE_IDLE;
}

uint16_t MaxHeatDotsAdj(uint16_t dots)
{
    uint16_t max_heat_dot;//新的最大值

    max_heat_dot = dots/(dots/(TP_MAX_HEAT_DOT+1)+1);//总加热点数加热次数
    if ((max_heat_dot+1) <= TP_MAX_HEAT_DOT)         //8个点的误差，引起多一次的加热
        max_heat_dot += 1;
    else
        max_heat_dot = TP_MAX_HEAT_DOT;
    return max_heat_dot;
}

static void TPDataShiftCntProc(uint8_t strobe_cnt)
{
	static uint8_t const Byte2DotNumTbl[] =
	{
		0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
		4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
	};
	uint8_t c;			// 当前字节
	uint8_t dot;		// 当前字节的点数
	uint8_t heat_cnt;	// 分开几次加热
	uint16_t max_dot;	// 加热点累加和
	uint16_t i,j,pt;		// 行缓冲区指针
    uint16_t max_heat_dots=0;

    for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt); i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
    {
	   c = TP_dot[tp.tail][pt];
	   dot = Byte2DotNumTbl[c];
       max_heat_dots += dot;
    }
    max_heat_dots = MaxHeatDotsAdj(max_heat_dots);

	memset(tp.heat_buf[0], 0, sizeof(tp.heat_buf[0]));
	for(i=0, pt=((LineDot/8/TP_MAX_HEAT_STROBE)*strobe_cnt), heat_cnt=0, max_dot=0; i<LineDot/8/TP_MAX_HEAT_STROBE; i++, pt++)
	{
		c = TP_dot[tp.tail][pt];
		dot = Byte2DotNumTbl[c];
		if((max_dot+dot)<=max_heat_dots)
		{
			max_dot += dot;
			tp.heat_buf[heat_cnt][pt] = c;
		}
		else
		{
            for (j=0; j<8; j++)
		    {
              c = TP_dot[tp.tail][pt] & (1<<(7-(j&0x07)));
              if(c)
              {
                 if((max_dot+1) <= max_heat_dots)
                 {
                     max_dot++;
                 }
                 else
                 {
                     heat_cnt++;
                     max_dot = 1;
                     memset((void *)tp.heat_buf[heat_cnt], 0, sizeof(tp.heat_buf[0]));
                 }
                 tp.heat_buf[heat_cnt][pt+j/8] |= c;
              }
		   }
        }

	}
	if(max_dot)
    {
       heat_cnt++;
	}

	tp.heat_max_cnt = heat_cnt;//每行最多加热的次数
	tp.heat_cnt = 0;
	// 计算具体的加速表来满足时间要求
	TPAdjustStepTime(heat_cnt,max_heat_dots);
}

static void TPDataShiftOut(uint8_t *p, uint16_t len)
{


        while (len--)
        {
            //Send byte through the SPI1 peripheral
            print_head_spi_send_byte(*p++);
           // Loop while DR register in not emplty

        }

        return;

}

static void TPDataDMAShiftToPrn(void)
{
    TPDataShiftOut(tp.heat_buf[tp.heat_cnt],ARRAY_SIZE(tp.heat_buf[0]));
}

static uint8_t TPCheckBuf(void)
{
	uint8_t ret;
	uint16_t feedmax;

   if(TPPrinterReady() !=  TRUE )
   {
    	 TPIntSetIdle();
	     ret = 0;
         return ret;
   }
	if (tp.head != tp.tail)
	{
		switch (TP_dot[tp.tail][LineDot/8])
		{
		case TP_CMD_PRINT_DOTLINE:
			TPDataShiftCntProc(0);		// 计算第一个加热行
#ifdef DEBUG_VER
			//debug_buffer[debug_cnt] = tp.heat;
			//debug_cnt++;
			//debug_buffer[debug_cnt] = tp.heat_max_cnt;
			//debug_cnt++;
			//debug_buffer[debug_cnt] = tp.feed_time[0];
			//debug_cnt++;
			//debug_buffer[debug_cnt] = tp.feed_time[1];
			//debug_cnt++;
			//debug_buffer[debug_cnt] = tp.feed_time[2];
			//debug_cnt++;
			//debug_buffer[debug_cnt] = tp.feed_time[3];
			//debug_cnt++;
#endif
			if(tp.heat_cnt < tp.heat_max_cnt)//本行还有数据需要加热
			{
				TPDataDMAShiftToPrn();		// 开始送数据到打印机
			}
			tp.heat_remain = 0;
			tp.feed_step = 0;
			tp.strobe_step = 0;
			tp.state = TPSTATE_HEAT_WITH_FEED;
			ret = 1;
			break;
		case TP_CMD_FEED:
			//tp.feedmax = TP_dot[tp.tail][0] | (TP_dot[tp.tail][1] << 8);
                        tp.feedmax = TP_dot[tp.tail][1];
                        tp.feedmax <<= 8;
                        tp.feedmax |= TP_dot[tp.tail][0];
			tp.tail = (tp.tail+1) & (ARRAY_SIZE(TP_dot)-1);
			tp.state = TPSTATE_FEED;
			ret = 2;
			break;
        case TP_CMD_FEED_TO_MARK:
            STROBE_0_OFF();
            STROBE_1_OFF();
			//feedmax = TP_dot[tp.tail][0] | (TP_dot[tp.tail][1] << 8);
                        feedmax = TP_dot[tp.tail][1];
                        feedmax <<= 8;
                        feedmax |= TP_dot[tp.tail][0];
                        tp.tail = (tp.tail+1) & (ARRAY_SIZE(TP_dot)-1);
			if(tp.markbefore > 0)	// 这次走纸有发现黑标
			{
			    if(tp.markbefore >= g_param.line_after_mark)	// 之前发现的黑标位置超过要求
				{
					tp.feedmax = feedmax;
					tp.state = TPSTATE_FEED_TO_MARK;
				}
				else	// 之前已经找到黑标并且没有超过允许范围
				{
					tp.feedmax = g_param.line_after_mark - tp.markbefore;
					tp.state = TPSTATE_FEED_AFTERMARK;
				}
			}
			else if(tp.markbefore == 0)		// 现在还停留在黑标位置
			{
				tp.feedmax = feedmax;
				tp.state = TPSTATE_FEED_FIND_MARK;
			}
			else	// 之前没有发现黑标，则开始找黑标
			{
				tp.feedmax = feedmax;
				tp.state = TPSTATE_FEED_TO_MARK;
			}
			ret = 2;
			break;
		default:	// 未知类型，属于严重错误
			tp.tail = (tp.tail+1) & (ARRAY_SIZE(TP_dot)-1);
			TPIntSetIdle();
			ret = 0;
			break;
		}
	}
	else
	{
        TPIntSetPreIdle();
		ret = 0;
	}
	return ret;
}

extern void TPISRProc(void)
{
	GPIOB->BSRR = GPIO_Pin_4; 
	switch (tp.state)
	{
	case TPSTATE_START: 	// start
		switch (TPCheckBuf())
		{
		case 1:		// 打印
			MOTOR_PWR_ON();
			TPSetTimeCnt(TPGetStepTime());	// set timer
			break;
		case 2:		// 走纸
			MOTOR_PWR_ON();
			TPSetTimeCnt(TPGetStepTime());	// set timer
			break;
		}
		break;
	case TPSTATE_HEAT_WITH_FEED:       // 开始马达步进
		TPForwardStep(1);
		// break;
	case TPSTATE_HEAT_WITHOUT_FEED:
		if(tp.heat_remain)			// 还要继续加热，每行刚开始加热或者一个步进内加热时间足够时此条件不成立
		{
			if(tp.feed_time[tp.feed_step] > tp.heat_remain)	// 当前步进的时间足够加热
			{
					TPSetTimeCnt(tp.heat_remain);	// 加热
					tp.feed_time[tp.feed_step] -= tp.heat_remain;
					tp.heat_remain = 0;
					tp.state = TPSTATE_HEAT_WITHOUT_FEED;
			}
			else			// 时间不够或者刚好，先加热剩余时间//如果时间不够则走一步后继续跳入上一个if,刚好则跳入else
			{
				TPSetTimeCnt(tp.feed_time[tp.feed_step]); // 加热剩余时间
				tp.heat_remain -= tp.feed_time[tp.feed_step];
				tp.state = TPSTATE_HEAT_WITH_FEED;
				tp.feed_step++;
			}
		}
		else
		{
		    if(tp.heat_cnt < tp.heat_max_cnt)	// 已经有数据被送给打印机
			{
				tp.heat_cnt++;
				LATCH_LOW();
				LATCH_HIGH();
				STROBE_0_ON();
				STROBE_1_ON();
				if(tp.heat_cnt < tp.heat_max_cnt)	// 还有数据需要送到打印机
				{
					TPDataDMAShiftToPrn();		// shift next heat data to printer
				}
				if(tp.feed_time[tp.feed_step] > tp.heat)	// 当前步进的时间足够加热
				{
					TPSetTimeCnt(tp.heat);	// 加热
					tp.feed_time[tp.feed_step] -= tp.heat;
					tp.state = TPSTATE_HEAT_WITHOUT_FEED;
				}
				else			// 时间不够或者刚好，先加热剩余时间
				{
					TPSetTimeCnt(tp.feed_time[tp.feed_step]);	// 加热剩余时间
					tp.heat_remain = tp.heat - tp.feed_time[tp.feed_step];
					tp.state = TPSTATE_HEAT_WITH_FEED;
					tp.feed_step++;
				}
			}
			else	// no any data need to print//本行加热次数完成
			{
				//trip2();
//strobe_off:
				STROBE_0_OFF(); 	// stop heat
				STROBE_1_OFF(); 	// stop heat
				TPSetTimeCnt(tp.feed_time[tp.feed_step]);   // 停止加热时间//最后一次加热会出现这种情况，加热次数完成还有剩余步进时间
				#if TP_MAX_STROBE_STEP>1
				if(tp.feed_step < (TP_MAX_STROBE_STEP-1))		// 每个加热Strobe中包含的步进数
				{
					tp.feed_step++;
					tp.state = TPSTATE_HEAT_WITH_FEED;
				}
				else	// 完成一个加热Strobe的控制
				#endif
				{
					tp.feed_step = 0;
					#if TP_MAX_HEAT_STROBE>1
				    if(tp.strobe_step < (TP_MAX_HEAT_STROBE-1))
					{
						tp.strobe_step++;
						TPDataShiftCntProc(tp.strobe_step);		// 计算下一个加热行
						if(tp.heat_cnt < tp.heat_max_cnt)
						{
							TPDataDMAShiftToPrn();		// 开始送数据到打印机
						}
						tp.state = TPSTATE_HEAT_WITH_FEED;
					}
					else	// 当前点行打印完成
					#endif
					{
						tp.tail = (tp.tail+1) & (ARRAY_SIZE(TP_dot)-1);
						switch(TPCheckBuf())
						{
						case 0: 	// no data
							//TPIntSetPreIdle();
							break;
						case 1:
						case 2:
						case 3:
							break;
						default:	// bug
							TPIntSetIdle();
							break;
						}
					}
				}
			}
		}
		break;
	case TPSTATE_FEED:
		if (TPFeedStep() == 0)
		{
			switch(TPCheckBuf())
			{
			case 0:		// no data
				//TPIntSetPreIdle();
				break;
			case 1:
			case 2:
			case 3:
				break;
			default:	// bug
				TPIntSetIdle();
				break;
			}
		}
		break;
     case TPSTATE_FEED_TO_MARK:
        TPSetTimeCnt(TPGetStepTime());  // set timer
        if(TPFeedStep())
        {
            if(TPPrinterMark() == FALSE)        // find mark
            {
                tp.state = TPSTATE_FEED_FIND_MARK;
            }
        }
        else
        {
            TPCheckBuf();
        }
        break;
    case TPSTATE_FEED_FIND_MARK:
        TPSetTimeCnt(TPGetStepTime());  // set timer
        if(TPFeedStep())
        {
            if(TPPrinterMark() == TRUE)     // space
            {
                //tp.feedmax = esc_sts.line_after_mark;
                tp.feedmax = g_param.line_after_mark;
                tp.state = TPSTATE_FEED_AFTERMARK;
            }
        }
        else
        {
            TPCheckBuf();
        }
        break;
    case TPSTATE_FEED_AFTERMARK:
        TPSetTimeCnt(TPGetStepTime());  // set timer
        if(TPFeedStep() == 0)
        {
            TPCheckBuf();
        }
        break;
    case TPSTATE_WAIT_TIME:
        if(--tp.repeat == 0 )
        {
           TPCheckBuf();
        }
        break;
	case TPSTATE_PRE_IDLE:
		TPSetTimeCnt(TIMER1_MS_TO_CNT(1.0));
		if(tp.feedmax)
		{
			tp.feedmax--;
			if(tp.feedmax & 0x01)
			{
				PRN_POWER_CHARGE();
			}
			else
			{
				PRN_POWER_DISCHARGE();
			}
		}
		else
		{
			tp.accel = 0;				// 下次需要退纸，重新开始缓启动
			switch(TPCheckBuf())
			{
			case 0:		// no data
			default:	// bug
				TPIntSetIdle();
				break;
			case 1:
			case 2:
			case 3:
				break;
			}
		}
		break;
	default:
		tp.state = TPSTATE_IDLE;
		//break;
	case TPSTATE_IDLE:
		TPIntSetIdle();
		break;
	}
	GPIOB->BRR = GPIO_Pin_4; 
}


void TIM3_IRQ_Handle(void)
{
	//trip1();
	//TIM3->CR1 &= ~0x0001;
    PRN_POWER_DISCHARGE();
    PRN_POWER_CHARGE();
    TPISRProc();
	//TIM3->CR1 |= 0x0001;
}

extern void TPSetSpeed(uint8_t speed)
{
	uint16_t const TPHeatTbl[] =
	{
		TIMER1_MS_TO_CNT(0.50), // 0
		TIMER1_MS_TO_CNT(0.60), // 1
		TIMER1_MS_TO_CNT(0.70), // 2
		TIMER1_MS_TO_CNT(0.80), // 3
		TIMER1_MS_TO_CNT(0.90),	// 4
		TIMER1_MS_TO_CNT(1.00), // 5
		TIMER1_MS_TO_CNT(1.20), // 6
		TIMER1_MS_TO_CNT(1.40), // 7
		TIMER1_MS_TO_CNT(1.50), // 8
		TIMER1_MS_TO_CNT(1.60), // 9                
		TIMER1_MS_TO_CNT(1.80), // 10
		TIMER1_MS_TO_CNT(2.00), // 11
		TIMER1_MS_TO_CNT(2.50), // 12
		TIMER1_MS_TO_CNT(3.00), // 13
		TIMER1_MS_TO_CNT(3.50), // 14
		TIMER1_MS_TO_CNT(4.00), // 15
		TIMER1_MS_TO_CNT(4.50), // 16
		TIMER1_MS_TO_CNT(5.00), // 17

	};
	if (speed < ARRAY_SIZE(TPHeatTbl))
	{
		tp.heat_setting = TPHeatTbl[speed];
	}
}



extern void SetDesity(void)
{
	 #if defined(LOW_5V_PRINT)
        TPSetSpeed(17);
     #else
	  	#if defined(HIGH_8V_PRINT)
		TPSetSpeed(6);
		#else
        TPSetSpeed(10);//10
        #endif
     #endif
}
//======================================================================================================
extern void Wake_up(void)
{
	tp.state = TPSTATE_WAIT_TIME;
    tp.accel = 0;
    tp.repeat =200;

	MOTOR_PWR_ON();
	//TIM_SetAutoreload(TIM3,TIMER1_MS_TO_CNT(5.00));
    TIM3->ARR = TIMER1_MS_TO_CNT(5.00);
	//TIM_SetCounter(TIM3,0);
	TIM3->CNT = 0;
	ENABLE_TIMER_INTERRUPT();
}
static void TPStart(void)
{
	uint8_t i;
    uint32_t delay;

	tp.state = TPSTATE_START;
	tp.accel = 0;

	for(i=0; i<6; i++)
	{
		PRN_POWER_CHARGE();
        delay = 30000;
        while(delay--);
		PRN_POWER_DISCHARGE();
        delay = 10000;
        while(delay--);
	}
	//TIM_SetAutoreload(TIM3,TpAccelerationSteps[0])
	TIM3->ARR = TpAccelerationSteps[0];
	PRN_POWER_CHARGE();
	//TIM_SetCounter(TIM3,0);
    TIM3->CNT = 0;
    ENABLE_TIMER_INTERRUPT();
}

extern void TPReStart(void)
{
	uint8_t i;
    uint32_t delay;

	if(tp.pause)
	{
		if (tp.head != tp.tail)
		{
			for(i=0; i<6; i++)
			{
				PRN_POWER_CHARGE();
                delay = 30000;
                while(delay--);
				PRN_POWER_DISCHARGE();
                delay = 10000;
                while(delay--);

			}
			PRN_POWER_CHARGE();

			tp.state = TPSTATE_START;
			tp.accel = 0;
			//TIM_SetAutoreload(TIM3,TpAccelerationSteps[0])
			TIM3->ARR = TpAccelerationSteps[0];
			PRN_POWER_CHARGE();
			//TIM_SetCounter(TIM3,0);
			TIM3->CNT = 0;
			ENABLE_TIMER_INTERRUPT();
		}
	}
}

//======================================================================================================
static void TPPrintCmdToBuf(uint8_t cmd, uint8_t *dot, uint8_t len)
{
	uint32_t head;
	head = (tp.head+1) & (ARRAY_SIZE(TP_dot)-1);
	while (head == tp.tail)
	{
		// 因为打印中断处理程序有可能在异常的情况下进入Idle状态，所以需要不断检查这个状态
        event_proc();
    }

    if (clr_all_dot == 1)
    {
        clr_all_dot = 0;
        return;
    }

	memcpy(TP_dot[tp.head & (ARRAY_SIZE(TP_dot)-1)], dot, len);
	TP_dot[tp.head][LineDot/8] = cmd;
	tp.head = head;
    if ((TPPrinterReady() && (tp.state == TPSTATE_IDLE)))//启动只有从IDE状态起来
    {
        TPStart();
    }
}
//======================================================================================================
extern void TPPrintLine(uint8_t *dot)
{
	TPPrintCmdToBuf(TP_CMD_PRINT_DOTLINE, dot, LineDot/8);
}
//======================================================================================================
extern void TPFeedLine(uint16_t line)
{
    #ifdef Half_Step
	line <<= 2;		// 一个点行等于4步
	#else
	line <<= 1;		// 一个点行等于2步
    #endif
	TPPrintCmdToBuf(TP_CMD_FEED, (uint8_t *)(&line), sizeof(line));
}
//======================================================================================================
extern void TPFeedToMark(uint16_t line)
{
	line <<= 1;		// 一个点行等于两步
	TPPrintCmdToBuf(TP_CMD_FEED_TO_MARK, (uint8_t *)(&line), sizeof(line));
}

extern uint32_t TPCheckBusy(void)
{
	if (tp.state == TPSTATE_IDLE)
	{
		return FALSE;
	}
	return TRUE;
}
//======================================================================================================
extern void TPFeedStart(void)
{
	if (tp.state == TPSTATE_IDLE)
	{
		TPFeedLine(500*8);		// 500mm
	}
}
//======================================================================================================
extern void TPFeedToMarkStart(void)
{
	if (tp.state == TPSTATE_IDLE)
	{
		TPFeedToMark(250*8);		// 250mm
	}
}
//======================================================================================================
extern void TPFeedStop(void)
{
	tp.feedmax = 0;		// interrupt will stop feed automaticcly
}
//======================================================================================================
extern uint8_t IsPrintBufEmpty(void)
{
	if(tp.head != tp.tail)	// have data
	{
		return 0;
	}
	else
	{
		return 1;
	}
}
static void TPPrintAsciiLine(char *buf, uint32_t len)
{
   PrintBufPushLine((uint8_t *)buf, len);
}
extern uint8_t IsPrinterIdle(void)
{
	if(tp.state == TPSTATE_IDLE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

extern void TPPrintTestPage(void)
{

    uint32_t len,i;
    char buf[64];

	debug_cnt = 0;
	current_channel = 0;
     PrintBufToZero();
    len = snprintf(buf, sizeof(buf),  "\n");
    TPPrintAsciiLine(buf,len);
#if 1
#if defined(PT486)
    len = snprintf(buf, sizeof(buf), "System: HJ_PT486_KT100\n");
#elif defined(PT488)
    len = snprintf(buf, sizeof(buf), "System: PT488_1MB1\n");
#elif defined(PT48D)
	len = snprintf(buf, sizeof(buf), "System: PT48D\n");
#elif defined(PT48F)
    len = snprintf(buf, sizeof(buf), "System: PT48F\n");
#elif defined(PT48G)
    len = snprintf(buf, sizeof(buf), "System: PT48G\n");
#endif
    TPPrintAsciiLine(buf,len);


    len = snprintf(buf, sizeof(buf), "Firmware:%d.%02d.%02d \n", VERSION_MAJOR, VERSION_MINOR,VERSION_TEST);
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf), "Build date: %s\n", __DATE__);
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf), "Build time: %s\n", __TIME__);
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "\n");
    TPPrintAsciiLine(buf,len);

    //len = snprintf(buf, sizeof(buf),  "[Uart Configure]\n");
    //TPPrintAsciiLine(buf,len);

    //len = snprintf(buf, sizeof(buf),  "baudrate : %ld\n", 115200);
    //TPPrintAsciiLine(buf,len);

    //len = snprintf(buf, sizeof(buf),  "flow ctrl : HW Flow Control\n");
    //TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  "[BT Module config]\n");
	TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  "%d BT Module Support\n",MAX_PT_CHANNEL);
	TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  "    Seq      |    Name    | Pin\n",MAX_PT_CHANNEL);
	TPPrintAsciiLine(buf,len);

	for (i = 0;i<MAX_PT_CHANNEL;i++)
	{
		len = snprintf(buf, sizeof(buf),  "BT Module(%d):|HJ Printer%d | 0000\n",i+1,i+1);
		TPPrintAsciiLine(buf,len);
	}

    len = snprintf(buf, sizeof(buf),  "\n");
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "[Install Fonts]\n");
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "ID  Font Name\n");
    TPPrintAsciiLine(buf,len);


    //if(esc_sts[current_channel].font_en == FONT_A_WIDTH)
    {
    len = snprintf(buf, sizeof(buf),  " 0  SYSTEM 12x24\n");
    TPPrintAsciiLine(buf,len);
    }
    //else
    {
	#if defined(FONTB_ASCII9X24)
	len = snprintf(buf, sizeof(buf),  " 1  SYSTEM 9x24\n");
	#else
    len = snprintf(buf, sizeof(buf),  " 1  SYSTEM 8x16\n");
	#endif
    TPPrintAsciiLine(buf,len);
    }
	len = snprintf(buf, sizeof(buf),  " 2  GBK 24x24\n");
	TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  " 3  GBK 16x16\n");
	TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "\n");
    TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "[ASCII Samples]\n");
    TPPrintAsciiLine(buf,len);

    for(i=0x20; i<0x80; i++)
    {
        PrintBufPushBytes(i);
    }
	len = snprintf(buf, sizeof(buf),  "\n\n");
	TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  "[GBK Samples]\n");
	TPPrintAsciiLine(buf,len);

	len = snprintf(buf, sizeof(buf),  "中文字库测试：简体字 繁體字\n");
	TPPrintAsciiLine(buf,len);
	len = snprintf(buf, sizeof(buf),  "\n");
	TPPrintAsciiLine(buf,len);

    len = snprintf(buf, sizeof(buf),  "Selftest Finished.\n");
    TPPrintAsciiLine(buf,len);
#endif

    len = snprintf(buf, sizeof(buf),  "\n\n\n\n\n");
    TPPrintAsciiLine(buf,len);


}


extern void TPSelfTest2(void)
{//打印斜线
	uint8_t dot[LineDot/8];
	uint32_t i,j;
	memset(dot, 0, sizeof(dot));
    for (i=0;i<100;i++)// 400
    {
        for (j=0;j<8;j++)
        {
            memset(dot,0x01<<j,sizeof(dot));
            TPPrintLine(dot);
        }
    }
}



void test_motor(void)
{
	int i,delay,cnt = 50000;
	MOTOR_PWR_ON();
	for(i=0; i<6; i++)
	{
		PRN_POWER_CHARGE();
		delay = 30000;
		while(delay--);
		PRN_POWER_DISCHARGE();
		delay = 10000;
		while(delay--);
	}

	while (cnt)
	{
		TPForwardStep(1);
		delay_ms(1);
                cnt--;
	}
	MOTOR_PWR_OFF();
}