/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V2.0.3
* Date               : 09/22/2008
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Type.h"
#include "hw_platform.h"
#include "stm32f10x_it.h"
#include "PaperDetect.h"
#include "TP.h"
#include "BT816.h"
#include "usb_int.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : NMIException
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMIException(void)
{
}


//然后在源程序里添加下面的函数代码: 
// hard fault handler in C,  
// with stack frame location as input parameter  
void hard_fault_handler_c(unsigned int * hardfault_args)  
{  
	unsigned int stacked_r0;  
	unsigned int stacked_r1;  
	unsigned int stacked_r2;  
	unsigned int stacked_r3;  
	unsigned int stacked_r12;  
	unsigned int stacked_lr;  
	unsigned int stacked_pc;  
	unsigned int stacked_psr;  
	//unsigned char	str[15];

	stacked_r0 = ((unsigned long) hardfault_args[0]);  
	stacked_r1 = ((unsigned long) hardfault_args[1]);  
	stacked_r2 = ((unsigned long) hardfault_args[2]);  
	stacked_r3 = ((unsigned long) hardfault_args[3]);  

	stacked_r12 = ((unsigned long) hardfault_args[4]);  
	stacked_lr = ((unsigned long) hardfault_args[5]);  
	stacked_pc = ((unsigned long) hardfault_args[6]);  
	stacked_psr = ((unsigned long) hardfault_args[7]);  

	//gui_SetFont(16);
	printf ("[Hard fault handler]\n");  
	//sprintf(str,"R0=%x",stacked_r0);
	//gui_TextOut(0,0,str,0,1);
	printf ("R0 = %x\n", stacked_r0); 
	
        
        //sprintf(str,"R1=%x",stacked_r1);
	//gui_TextOut(0,20,str,0,1);
	printf ("R1 = %x\n", stacked_r1);
	//sprintf(str,"R2=%x",stacked_r2);
	//gui_TextOut(0,40,str,0,1);;  
	printf ("R2 = %x\n", stacked_r2);
	//sprintf(str,"R3=%x",stacked_r3);
	//gui_TextOut(0,60,str,0,1);
	printf ("R3 = %x\n", stacked_r3);
	//sprintf(str,"R12=%x",stacked_r12);
	//gui_TextOut(0,80,str,0,1);
	printf ("R12 = %x\n", stacked_r12);  
	//sprintf(str,"LR=%x",stacked_lr);
	//gui_TextOut(0,100,str,0,1);
	printf ("LR = %x\n", stacked_lr);
	//sprintf(str,"PC=%x",stacked_pc);
	//gui_TextOut(0,120,str,0,1);
	printf ("PC = %x\n", stacked_pc);
	//sprintf(str,"PSR=%x",stacked_psr);
	//gui_TextOut(0,140,str,0,1);
	printf ("PSR = %x\n", stacked_psr);  
	printf ("BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));  
	printf ("CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));  
	printf ("HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));  
	printf ("DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));  
	printf ("AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));  


	while(1) 
	{ 
		;; 
	} 

}  
/*******************************************************************************
* Function Name  : HardFaultException
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFaultException(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
 // while (1)
 // {
	// // gui_TextOut(0,0,"HardFault!",0);
	////  while(1);
 // }
	HARDFAULT_ISR_HANDLE_d();
}

//HardFault_Handler\ 
//PROC 
//;EXPORT  HardFault_Handler         [WEAK] 
//;B                  . 
//IMPORT hard_fault_handler_c  
//TST LR, #4  
//ITE EQ  
//MRSEQ R0, MSP  
//MRSNE R0, PSP  
//B hard_fault_handler_c  
//ENDP 

/*******************************************************************************
* Function Name  : MemManageException
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManageException(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : BusFaultException
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFaultException(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : UsageFaultException
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFaultException(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/*******************************************************************************
* Function Name  : DebugMonitor
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMonitor(void)
{
}

/*******************************************************************************
* Function Name  : SVCHandler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVCHandler(void)
{
}

/*******************************************************************************
* Function Name  : PendSVC
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSVC(void)
{
}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTickHandler(void)
{ 
	SysTick_IRQ_Handle();
}

/*******************************************************************************
* Function Name  : WWDG_IRQHandler
* Description    : This function handles WWDG interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WWDG_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles PVD interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PVD_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TAMPER_IRQHandler
* Description    : This function handles Tamper interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TAMPER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTC_IRQHandler
* Description    : This function handles RTC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : FLASH_IRQHandler
* Description    : This function handles Flash interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RCC_IRQHandler
* Description    : This function handles RCC interrupt request. 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI0_IRQHandler
* Description    : This function handles External interrupt Line 0 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI0_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : EXTI1_IRQHandler
* Description    : This function handles External interrupt Line 1 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI1_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : EXTI2_IRQHandler
* Description    : This function handles External interrupt Line 2 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI3_IRQHandler
* Description    : This function handles External interrupt Line 3 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI3_IRQHandler(void)
{

}

/*******************************************************************************
* Function Name  : EXTI4_IRQHandler
* Description    : This function handles External interrupt Line 4 request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel1_IRQHandler
* Description    : This function handles DMA1 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	if (DMA_GetITStatus(DMA1_IT_TC2))
	{
		DMA_Cmd(DMA1_Channel2, DISABLE);
	}
	/* clear DMA flag */
	DMA_ClearFlag(DMA1_FLAG_TC2 | DMA1_FLAG_TE2);
#endif
}

/*******************************************************************************
* Function Name  : DMA1_Channel3_IRQHandler
* Description    : This function handles DMA1 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel4_IRQHandler
* Description    : This function handles DMA1 Channel 4 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel4_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (DMA_GetITStatus(DMA1_IT_TC4))
	{
		DMA_Cmd(DMA1_Channel4, DISABLE);
	}
	/* clear DMA flag */
	DMA_ClearFlag(DMA1_FLAG_TC4 | DMA1_FLAG_TE4);
#endif
}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel6_IRQHandler
* Description    : This function handles DMA1 Channel 6 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA1_Channel7_IRQHandler
* Description    : This function handles DMA1 Channel 7 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel7_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (DMA_GetITStatus(DMA1_IT_TC7))
	{
		DMA_Cmd(DMA1_Channel7, DISABLE);
	}
	/* clear DMA flag */
	DMA_ClearFlag(DMA1_FLAG_TC7 | DMA1_FLAG_TE7);
#endif
}

/*******************************************************************************
* Function Name  : ADC1_2_IRQHandler
* Description    : This function handles ADC1 and ADC2 global interrupts requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC1_2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USB_HP_CAN_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN_TX_IRQHandler(void)
{
  CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN_RX0_IRQHandler(void)
{
  USB_Istr();
}

/*******************************************************************************
* Function Name  : CAN_RX1_IRQHandler
* Description    : This function handles CAN RX1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_RX1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : CAN_SCE_IRQHandler
* Description    : This function handles CAN SCE interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void CAN_SCE_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : EXTI9_5_IRQHandler
* Description    : This function handles External lines 9 to 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_BRK_IRQHandler
* Description    : This function handles TIM1 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_UP_IRQHandler
* Description    : This function handles TIM1 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_TRG_COM_IRQHandler
* Description    : This function handles TIM1 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM1_CC_IRQHandler
* Description    : This function handles TIM1 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM1_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : This function handles TIM2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM2_IRQHandler(void)
{
    TIM2_UpdateISRHandler();
}

/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : This function handles TIM3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM3_IRQ_Handle();
		//TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		 TIM3->SR = (u16)~TIM_IT_Update;
	}
}

/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : This function handles TIM4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_EV_IRQHandler
* Description    : This function handles I2C1 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C1_ER_IRQHandler
* Description    : This function handles I2C1 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C1_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_EV_IRQHandler
* Description    : This function handles I2C2 Event interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_EV_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : I2C2_ER_IRQHandler
* Description    : This function handles I2C2 Error interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void I2C2_ER_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI1_IRQHandler
* Description    : This function handles SPI1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI2_IRQHandler
* Description    : This function handles SPI2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART1_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	unsigned int temp = 0;    
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
	{  
		//USART_ClearFlag(USART1,USART_IT_IDLE);  
		temp = USART1->SR;  
		temp = USART1->DR; //清USART_IT_IDLE标志  
		DMA_Cmd(DMA1_Channel5,DISABLE);  

		temp = BT816_RES_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel5);  

		BT816_Channel1_RxISRHandler(BT816_recbuffer[BT1_MODULE],temp); 

		//设置传输数据长度  
		//DMA1_Channel5->CNDTR = BT816_RES_BUFFER_LEN; 


		//打开DMA  
		DMA_Cmd(DMA1_Channel5,ENABLE);  
	}  
	else if (USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART1);
	}
#endif
}

/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : This function handles USART2 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART2_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	unsigned int temp = 0;    
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  
	{  
		//USART_ClearFlag(USART2,USART_IT_IDLE);  
		temp = USART2->SR;  
		temp = USART2->DR; //清USART_IT_IDLE标志  
		DMA_Cmd(DMA1_Channel6,DISABLE);  

		temp = BT816_RES_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel6);  

		BT816_Channel2_RxISRHandler(BT816_recbuffer[BT2_MODULE],temp); 

		//设置传输数据长度  
		//DMA1_Channel6->CNDTR = BT816_RES_BUFFER_LEN; 


		//打开DMA  
		DMA_Cmd(DMA1_Channel6,ENABLE);  
	}  
	else if (USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART2);
	}
#endif
}

/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : This function handles USART3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USART3_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT3_MODULE)
	unsigned int temp = 0;    
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  
	{  
		//USART_ClearFlag(USART3,USART_IT_IDLE);  
		temp = USART3->SR;  
		temp = USART3->DR; //清USART_IT_IDLE标志  
		DMA_Cmd(DMA1_Channel3,DISABLE);  

		temp = BT816_RES_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Channel3);  

		BT816_Channel3_RxISRHandler(BT816_recbuffer[BT3_MODULE],temp); 

		//设置传输数据长度  
		//DMA1_Channel3->CNDTR = BT816_RES_BUFFER_LEN; 


		//打开DMA  
		DMA_Cmd(DMA1_Channel3,ENABLE);  
	}  
	else if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(USART3);
	}
#endif
}

/*******************************************************************************
* Function Name  : EXTI15_10_IRQHandler
* Description    : This function handles External lines 15 to 10 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : RTCAlarm_IRQHandler
* Description    : This function handles RTC Alarm interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RTCAlarm_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : USBWakeUp_IRQHandler
* Description    : This function handles USB WakeUp interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USBWakeUp_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_BRK_IRQHandler
* Description    : This function handles TIM8 Break interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_BRK_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_UP_IRQHandler
* Description    : This function handles TIM8 overflow and update interrupt 
*                  request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_UP_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_TRG_COM_IRQHandler
* Description    : This function handles TIM8 Trigger and commutation interrupts 
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_TRG_COM_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM8_CC_IRQHandler
* Description    : This function handles TIM8 capture compare interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM8_CC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : ADC3_IRQHandler
* Description    : This function handles ADC3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ADC3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : FSMC_IRQHandler
* Description    : This function handles FSMC global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void FSMC_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM5_IRQHandler
* Description    : This function handles TIM5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : SPI3_IRQHandler
* Description    : This function handles SPI3 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : This function handles UART4 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART4_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	unsigned int temp = 0;    
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)  
	{  
		DMA_Cmd(DMA2_Channel3,DISABLE);
		//USART_ClearFlag(USART1,USART_IT_IDLE);  
		temp = UART4->SR;  
		temp = UART4->DR; //清USART_IT_IDLE标志  

		temp = BT816_RES_BUFFER_LEN - DMA_GetCurrDataCounter(DMA2_Channel3);  

		BT816_Channel4_RxISRHandler(BT816_recbuffer[BT4_MODULE],temp); 

		//设置传输数据长度  
		//DMA2_Channel3->CNDTR = BT816_RES_BUFFER_LEN; 


		//打开DMA  
		DMA_Cmd(DMA2_Channel3,ENABLE);  
	}  
	else if (USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET)
	{
		USART_ReceiveData(UART4);
	}
#endif
}

/*******************************************************************************
* Function Name  : UART5_IRQHandler
* Description    : This function handles UART5 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UART5_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM6_IRQHandler
* Description    : This function handles TIM6 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM6_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : TIM7_IRQHandler
* Description    : This function handles TIM7 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM7_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel1_IRQHandler
* Description    : This function handles DMA2 Channel 1 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel1_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel2_IRQHandler
* Description    : This function handles DMA2 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel2_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel3_IRQHandler
* Description    : This function handles DMA2 Channel 3 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel3_IRQHandler(void)
{
}

/*******************************************************************************
* Function Name  : DMA2_Channel4_5_IRQHandler
* Description    : This function handles DMA2 Channel 4 and DMA2 Channel 5
*                  interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA2_Channel4_5_IRQHandler(void)
{
#if(BT_MODULE_CONFIG & USE_BT4_MODULE)
	if (DMA_GetITStatus(DMA2_IT_TC5))
	{
		DMA_Cmd(DMA2_Channel5, DISABLE);
	}
	/* clear DMA flag */
	DMA_ClearFlag(DMA2_FLAG_TC5 | DMA2_FLAG_TE5);
#endif
}

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/
