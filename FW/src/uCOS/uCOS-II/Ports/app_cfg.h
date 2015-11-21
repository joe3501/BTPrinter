/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      APPLICATION CONFIGURATION
*
*                                     ST Microelectronics STM32
*                                              with the
*                                   IAR STM32-SK Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : Brian Nagel
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  uC_PROBE_OS_PLUGIN              DEF_ENABLED            /* DEF_ENABLED = Present, DEF_DISABLED = Not Present        */
//#define  uC_PROBE_COM_MODULE             DEF_ENABLED
//#define  uC_LCD_MODULE                   DEF_ENABLED

/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

//#define  APP_TASK_START_PRIO                   3
//#define  APP_TASK_KBD_PRIO                     4
//#define  APP_TASK_USER_IF_PRIO                 5
//#define  APP_TASK_PROBE_STR_PRIO               6

#define  OS_PROBE_TASK_PRIO                    20
#define  OS_PROBE_TASK_ID                      20

#define  OS_TASK_TMR_PRIO              (OS_LOWEST_PRIO - 2)

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/


//#define  APP_TASK_START_STK_SIZE             128
//#define  APP_TASK_USER_IF_STK_SIZE           128
//#define  APP_TASK_KBD_STK_SIZE               128
//#define  APP_TASK_PROBE_STR_STK_SIZE         128

#define  OS_PROBE_TASK_STK_SIZE              128

/*
*********************************************************************************************************
*                               uC/Probe plug-in for uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define  OS_PROBE_TASK                         1                /* Task will be created for uC/Probe OS Plug-In             */
#define  OS_PROBE_TMR_32_BITS                  0                /* uC/Probe OS Plug-In timer is a 16-bit timer              */
#define  OS_PROBE_TIMER_SEL                    2
#define  OS_PROBE_HOOKS_EN                     1

/*
*********************************************************************************************************
*                                          uC/LCD CONFIGURATION
*********************************************************************************************************
*/

//#define  DISP_BUS_WIDTH                        4                /* LCD controller is accessed with a 4 bit bus              */




#endif
