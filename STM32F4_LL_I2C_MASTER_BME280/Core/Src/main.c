/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "BME280/bme280.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// bme280 variables
float temperature;
float humidity;
float pressure;

struct bme280_dev dev;
struct bme280_data comp_data;
int8_t rslt;

// support printf
int __io_putchar (int ch)
{
  while (!LL_USART_IsActiveFlag_TXE(USART2));
  LL_USART_TransmitData8(USART2, (char)ch);
  return ch;
}

// Master transmitter
int8_t I2C_MasterTx(uint8_t devAddr, uint8_t* buffer, uint16_t len, uint16_t ms)
{
  // timeout counter
  uint16_t cnt = 0;

  // 1. start condition
  LL_I2C_GenerateStartCondition(I2C1);

  // 2. check start bit flag
  while(!LL_I2C_IsActiveFlag_SB(I2C1))
  {
    LL_mDelay(1);
    if(cnt++ > ms) return -1;
  }

  // 3. write device address (WRITE)
  LL_I2C_TransmitData8(I2C1, (devAddr << 1) | 0x00);

  // 4. wait address sent
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
  {
    LL_mDelay(1);
    if(cnt++ > ms) return -1;
  }

  // 5. clear ADDR flag
  LL_I2C_ClearFlag_ADDR(I2C1);

  // 6. check TXE flag & write data
  for(int i=0; i < len; i++)
  {
    while(!LL_I2C_IsActiveFlag_TXE(I2C1))
    {
      LL_mDelay(1);
      if(cnt++ > ms) return -1;
    }

    LL_I2C_TransmitData8(I2C1, buffer[i]);
  }

  // 7. wait BTF flag (TXE flag set & empty DR condition)
  while(!LL_I2C_IsActiveFlag_BTF(I2C1))
  {
    LL_mDelay(1);
    if(cnt++ > ms) return -1;
  }

  // 8. stop condition
  LL_I2C_GenerateStopCondition(I2C1);

  return 0;
}

// Master receiver
int8_t I2C_MasterRx(uint8_t devAddr, uint8_t* buffer, uint8_t len, uint16_t ms)
{
  // timeout counter
  uint16_t cnt = 0;

  // 1. fast NACK configuration when receiving single byte.
  if(len == 1)
  {
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
  }
  else
  {
    LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_ACK);
  }

  // 2. start condition
  LL_I2C_GenerateStartCondition(I2C1);

  // 3. check start bit flag
  while(!LL_I2C_IsActiveFlag_SB(I2C1))
  {
    LL_mDelay(1);
    if(cnt++ > ms) return -1;
  }

  // 4. write device address (READ)
  LL_I2C_TransmitData8(I2C1, (devAddr << 1) | 0x01);

  // 5. wait address sent
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1))
  {
    LL_mDelay(1);
    if(cnt++ > ms) return -1;
  }

  // 6. clear ADDR flag
  LL_I2C_ClearFlag_ADDR(I2C1);

  // 7. check RXNE flag & read data
  for(int i = 0; i < len; i++)
  {
    // 8. NACK at last byte
    if(i == len - 1)
    {
      LL_I2C_AcknowledgeNextData(I2C1, LL_I2C_NACK);
    }

    while(!LL_I2C_IsActiveFlag_RXNE(I2C1))
    {
      LL_mDelay(1);
      if(cnt++ > ms) return -1;
    }

    buffer[i] = LL_I2C_ReceiveData8(I2C1);
  }

  // 9. stop condition
  LL_I2C_GenerateStopCondition(I2C1);

  return 0;
}

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  // send register
  if(I2C_MasterTx(id, &reg_addr, 1, 50) < 0) return -1;
  // read data
  if(I2C_MasterRx(id, data, len, 50) < 0) return -1;
  return 0;
}

void user_delay_ms(uint32_t period)
{
  LL_mDelay(period);
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  int8_t *buf;
  buf = malloc(len +1);
  buf[0] = reg_addr;
  memcpy(buf +1, data, len);

  //send register + data
  if(I2C_MasterTx(id, (uint8_t*)buf, len + 1, 50) < 0)
  {
    free(buf);
    return -1;
  }

  free(buf);
  return 0;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // free I2C busy line
  while(LL_I2C_IsActiveFlag_BUSY(I2C1))
  {
    printf("I2C1 is busy. Toggle SCL to free I2C1.\r\n");

    // change SCL to GPIO output
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // toggle SCL
    for(int i =0; i < 50; i++)
    {
      LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_8);
      LL_mDelay(1);
    }

    LL_mDelay(100);
  }

  // initialize BME280
  dev.dev_id = BME280_I2C_ADDR_PRIM;
  dev.intf = BME280_I2C_INTF;
  dev.read = user_i2c_read;
  dev.write = user_i2c_write;
  dev.delay_ms = user_delay_ms;

  rslt = bme280_init(&dev);
  if(rslt != BME280_OK)
  {
    printf("Fail to initialize BME280.\r\n");
    while(1);
  }

  // configure BME280
  dev.settings.osr_h = BME280_OVERSAMPLING_1X;
  dev.settings.osr_p = BME280_OVERSAMPLING_16X;
  dev.settings.osr_t = BME280_OVERSAMPLING_2X;
  dev.settings.filter = BME280_FILTER_COEFF_16;
  rslt = bme280_set_sensor_settings(BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL, &dev);
  if(rslt != BME280_OK)
  {
    printf("Fail to configure BME280.\r\n");
    while(1);
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // configure FORCED mode, turn into SLEEP mode after sensing
    bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
    dev.delay_ms(40);
    // measure sensors
    rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
    if(rslt == BME280_OK)
    {
      temperature = comp_data.temperature / 100.0;      /* ??C  */
      humidity = comp_data.humidity / 1024.0;           /* %   */
      pressure = comp_data.pressure / 10000.0;          /* hPa */

      printf("Temp: %3.1f, Humidity:%3.1f, Pressure:%4.1f\r\n", temperature, humidity, pressure);
    }

    LL_GPIO_TogglePin(GPIOD, LL_GPIO_PIN_15);
    LL_mDelay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

  /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

  /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

  /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(168000000);
  LL_SetSystemCoreClock(168000000);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
