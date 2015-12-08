#include "Type.h"
#include "stm32f10x_lib.h" 
#include "PaperDetect.h"
#include "basic_fun.h"
#include "Esc_p.h"
#include "Event.h"
#include "KeyScan.h"
#include "uart.h"

uint8_t printersts,papercnt,platencnt,bm_cnt;

uint16_t		AD_Value[10][2];		//�ֱ���ȱֽ����ADֵ���¶ȼ���ADֵ


#define PAPER_SNS          (1ul<<0)
#define PAPER_READY        (1ul<<1)
#define BLACKMARKR_FLAG    (1ul<<3)


#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X0130UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_LOW)


#define PAPER_SNS_VALUE		((AD_Value[0][PAPER_SNS_OFFSET]+AD_Value[1][PAPER_SNS_OFFSET]+AD_Value[2][PAPER_SNS_OFFSET]+AD_Value[3][PAPER_SNS_OFFSET]\
							+AD_Value[4][PAPER_SNS_OFFSET]+AD_Value[5][PAPER_SNS_OFFSET]+AD_Value[6][PAPER_SNS_OFFSET]+AD_Value[7][PAPER_SNS_OFFSET]\
							+AD_Value[8][PAPER_SNS_OFFSET]+AD_Value[9][PAPER_SNS_OFFSET])/10)


#define BMSNS()     ((PAPER_SNS_VALUE < ((uint16_t)AD_BLACKMARK_HIGH))&&(PAPER_SNS_VALUE > ((uint16_t)AD_BLACKMARK_LOW))?1:0)
#define PAPERSNS()  ((PAPER_SNS_VALUE<PAPER_AD_LTHRESHOLD)?1:0)




//======================================================================================================
void TPPaperSNSInit(void)
{
	int i;
	GPIO_InitTypeDef		gpio_init;
	ADC_InitTypeDef		adc_init;
	DMA_InitTypeDef		DMA_InitStructure;


	//����ADC��IO
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);		//72M/6 = 12M; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

	//PA.6 -- TEMP_SNS  &  PC.0 -- PEPER_SNS
	gpio_init.GPIO_Pin  = GPIO_Pin_6;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &gpio_init);

	gpio_init.GPIO_Pin  = GPIO_Pin_0;
	gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
	gpio_init.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &gpio_init);


	//����ADC��2��ͨ��
	adc_init.ADC_Mode               = ADC_Mode_Independent;		//
	adc_init.ADC_ScanConvMode       = ENABLE;
	adc_init.ADC_ContinuousConvMode = ENABLE;
	adc_init.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
	adc_init.ADC_DataAlign          = ADC_DataAlign_Right;
	adc_init.ADC_NbrOfChannel       = 2;
	ADC_Init(ADC1, &adc_init);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 2, ADC_SampleTime_239Cycles5);
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
	DMA_DeInit(DMA1_Channel1); //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//����DMA���Զ���ADת��������浽�ڴ� 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA����ADC����ַ 
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&AD_Value; //DMA�ڴ����ַ  
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //�ڴ���Ϊ���ݴ����Ŀ�ĵ� 
	DMA_InitStructure.DMA_BufferSize = 2*10; //DMAͨ����DMA����Ĵ�С  
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ�Ĵ������� 
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //�ڴ��ַ�Ĵ������� 
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //���ݿ��Ϊ16λ  
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //���ݿ��Ϊ16λ  
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //������ѭ������ģʽ  
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ�  
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��  
	DMA_Init(DMA1_Channel1, &DMA_InitStructure); //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ�� 

	ADC_DMACmd(ADC1,ENABLE);
	DMA_Cmd(DMA1_Channel1,ENABLE);

	//����SYSTICK��ʱ����ȱֽ����������SYSTICK��ʱ�����жϴ�����������ڵĵ���ִ��
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//9M��tickƵ��
	SysTick_SetReload(90000);			//�ж�Ƶ��Ϊ10ms
	NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick,1,4);
	SysTick_ITConfig(ENABLE);
	SysTick_CounterCmd(DISABLE);

	for (i = 0; i<MAX_PRINT_CHANNEL;i++)
	{
		esc_sts[i].status4=0;
	}

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);			//��ʼת��
}

void PaperStartSns(void)
{
	int i;

    	if(!PAPERSNS()) //
       {
            printersts &= ~PAPER_SNS;
            printersts |= PAPER_READY;
			for (i = 0; i<MAX_PRINT_CHANNEL;i++)
			{
				esc_sts[i].status4 &=~(0x03<<5);
			}
       }
       else // paper out
       {
            printersts |= PAPER_SNS;
            printersts &= ~PAPER_READY;
			for (i = 0; i<MAX_PRINT_CHANNEL;i++)
			{
				esc_sts[i].status4 |= (0x03<<5);
			}
       }

		SysTick_CounterCmd(ENABLE);

}

void TPBMSNSDetect(void)
{
        if(BMSNS())         //?????????????1
        {
            if((printersts & (1 << 2)) == 0)        // previous head out
            {
                printersts |= (1<<2);
                bm_cnt = 1;  //10ms

            }
            else if(bm_cnt )
            {
                if((--bm_cnt ) == 0)
                {
                    if((printersts & BLACKMARKR_FLAG) == 0)
                    {
                        printersts |= BLACKMARKR_FLAG; // set head
                        event_post(evtBmDetect);
                    }
                }
            }
        }
        else
        {
            if(printersts & (1 << 2))   // previous head
            {
                printersts &= ~(1<<2);
                bm_cnt = 1;
            }
            else if(bm_cnt)
            {
                if((--bm_cnt) == 0)
                {
                    if((printersts & BLACKMARKR_FLAG))
                    {
                        printersts &= ~BLACKMARKR_FLAG;    // set head
                        event_post(evtBmNotDetect);
                    }
                }
            }

        }
}

void TPPaperSNSDetect(uint8_t c)//488?????,486?????
{
	if(!c)
	{
		if((printersts & (1 << 0)) == 0)		// previous paper out
		{
			printersts |= (1<<0);  //xxx1
			papercnt = 2;	// 20ms
		}
		else if(papercnt)
		{
			if((--papercnt) == 0)
			{
				if((printersts & (1 << 1)) == 0)
				{
					printersts |= PAPER_READY;	// set paper in //xx1x
					event_post(evtPaperIn);


				}
			}
		}
	}
	else
	{
		if(printersts & (1 << 0))	// previous paper in
		{
			printersts &= ~(1<<0);  //xxx0
			papercnt = 2;	// 20ms
		}
		else if(papercnt)
		{
			if((--papercnt) == 0)
			{
				if((printersts & (1 << 1)))
				{
					printersts &= ~(1 << 1);	// set paper out xx0x
					event_post(evtPaperOut);

				}
			}
		}
	}

}
//======================================================================================================
//=================================================================================================
uint8_t TPPrinterMark(void)
{
	if(printersts & BLACKMARKR_FLAG)
	{
         return FALSE;
    }
	else
	{
         return TRUE;
    }
}
//======================================================================================================

uint8_t TPPrinterReady(void)
{
   // return TRUE;

	if(printersts & PAPER_READY)
	{
		return TRUE;
	}
	else
    {
	    return FALSE;
	}

}

uint8_t TPPrinterReady_ext(void)
{
	// return TRUE;

	if(PAPERSNS())
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}

}

uint8_t TPPaperReady(void)
 {

	if(printersts & PAPER_READY)
	{
		return TRUE;
	}
	else
    {
	    return FALSE;
	}

}

//================================================================================================
void SysTick_IRQ_Handle(void)
{
	int i;
	KeyScanProc();
	TPBMSNSDetect();
	TPPaperSNSDetect( PAPERSNS());
	for (i = 0; i < MAX_BT_CHANNEL;i++)
	{
		BT816_connect_status(i);
	}
#ifdef HW_VER_LCD
	//LCD_Refresh();
#else
	//LedScanProc();

#endif
	return;
}







