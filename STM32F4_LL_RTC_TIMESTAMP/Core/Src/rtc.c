/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   This file provides code for the configuration
 *          of the RTC instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>
/* USER CODE END 0 */

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  LL_RTC_InitTypeDef RTC_InitStruct = {0};
  LL_RTC_TimeTypeDef RTC_TimeStruct = {0};
  LL_RTC_DateTypeDef RTC_DateStruct = {0};

  if(LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSI)
  {
    LL_RCC_ForceBackupDomainReset();
    LL_RCC_ReleaseBackupDomainReset();
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSI);
  }

  /* Peripheral clock enable */
  LL_RCC_EnableRTC();

  /* RTC interrupt Init */
  NVIC_SetPriority(TAMP_STAMP_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TAMP_STAMP_IRQn);

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC and set the Time and Date
   */
  RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
  RTC_InitStruct.AsynchPrescaler = 127;
  RTC_InitStruct.SynchPrescaler = 255;
  LL_RTC_Init(RTC, &RTC_InitStruct);

  // remove useless code
  //LL_RTC_SetAsynchPrescaler(RTC, 127);
  //LL_RTC_SetSynchPrescaler(RTC, 255);

  /** Initialize RTC and set the Time and Date */
  if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) != 0x32F2)
  {
    //Backup register is cleared by POR
    printf("Backup register is cleared by POR. Reconfiguring RTC.\r\n");

    // cubemx code generation is incorrect
    RTC_TimeStruct.Hours = 0x12;
    RTC_TimeStruct.Minutes = 0x12;
    RTC_TimeStruct.Seconds = 0x12;
    LL_RTC_TIME_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_TimeStruct);
    RTC_DateStruct.WeekDay = LL_RTC_WEEKDAY_MONDAY;
    RTC_DateStruct.Month = LL_RTC_MONTH_APRIL;
    RTC_DateStruct.Day = 0x18;
    RTC_DateStruct.Year = 0x22;
    LL_RTC_DATE_Init(RTC, LL_RTC_FORMAT_BCD, &RTC_DateStruct);
    LL_RTC_BAK_SetRegister(RTC,LL_RTC_BKP_DR0,0x32F2);
  }

  /** Initialize RTC and set the Time and Date */
  // remove useless code
  //if(LL_RTC_BAK_GetRegister(RTC, LL_RTC_BKP_DR0) != 0x32F2){
  //  LL_RTC_BAK_SetRegister(RTC,LL_RTC_BKP_DR0,0x32F2);
  //}

  /** Enable the TimeStamp */
  LL_RTC_TS_Enable(RTC);
  LL_RTC_TS_SetActiveEdge(RTC, LL_RTC_TIMESTAMP_EDGE_RISING);
  LL_RTC_EnableIT_TS(RTC);
  /* USER CODE BEGIN RTC_Init 2 */

  // cubemx code generation is incorrect.
  // added time stamp configuration code

  // disable write protection
  LL_RTC_DisableWriteProtection(RTC);

  LL_RTC_TS_Enable(RTC);
  LL_RTC_TS_SetActiveEdge(RTC, LL_RTC_TIMESTAMP_EDGE_RISING);
  LL_RTC_EnableIT_TS(RTC);

  // enable EXTI21 for time stamp. refer to stm32f4x7 reference manual 26.5 RTC interrupts
  LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_21);
  LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_21);

  // enable write protection
  LL_RTC_EnableWriteProtection(RTC);

  /* USER CODE END RTC_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
