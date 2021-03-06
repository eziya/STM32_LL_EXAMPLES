/*
 * u8g2_stm32f4.c
 *
 *  Created on: 2022. 4. 05.
 *      Author: KIKI
 */

#include "main.h"
#include "u8g2/u8g2.h"

#define DEVICE_ADDRESS 	0x3C
#define TX_TIMEOUT		  100

// I2C master transmitter
void I2C_MasterTx(uint8_t devAddr, uint8_t* buffer, uint16_t len)
{
  // 1. start condition
  LL_I2C_GenerateStartCondition(I2C1);

  // 2. check start bit flag
  while(!LL_I2C_IsActiveFlag_SB(I2C1));

  // 3. write device address (WRITE)
  LL_I2C_TransmitData8(I2C1, (devAddr << 1) | 0x00);

  // 4. wait address sent
  while(!LL_I2C_IsActiveFlag_ADDR(I2C1));

  // 5. clear ADDR flag
  LL_I2C_ClearFlag_ADDR(I2C1);

  // 6. check TXE flag & write data
  for(int i=0; i < len; i++)
  {
    while(!LL_I2C_IsActiveFlag_TXE(I2C1));
    LL_I2C_TransmitData8(I2C1, buffer[i]);
  }

  // 7. wait BTF flag (TXE flag set & empty DR condition)
  while(!LL_I2C_IsActiveFlag_BTF(I2C1));

  // 8. stop condition
  LL_I2C_GenerateStopCondition(I2C1);
}

// u8g2 gpio & delay control callback
uint8_t u8x8_stm32_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* STM32 supports HW SPI, Remove unused cases like U8X8_MSG_DELAY_XXX & U8X8_MSG_GPIO_XXX */
  switch(msg)
  {
  case U8X8_MSG_GPIO_AND_DELAY_INIT:
    /* Insert codes for initialization */
    break;
  case U8X8_MSG_DELAY_MILLI:
    /* ms Delay */
    LL_mDelay(arg_int);
    break;
  case U8X8_MSG_GPIO_CS:
    /* Insert codes for SS pin control */
    if(arg_int)
    {
      LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_9);
    }
    else
    {
      LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_9);
    }
    break;
  case U8X8_MSG_GPIO_DC:
    /* Insert codes for DC pin control */
    if(arg_int)
    {
      LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_10);
    }
    else
    {
      LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_10);
    }
    break;
  case U8X8_MSG_GPIO_RESET:
    /* Insert codes for RST pin control */
    if(arg_int)
    {
      LL_GPIO_SetOutputPin(GPIOD, LL_GPIO_PIN_11);
    }
    else
    {
      LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_11);
    }
    break;
  }
  return 1;
}

uint8_t u8x8_byte_stm32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t* data = arg_ptr;

  switch(msg) {
  case U8X8_MSG_BYTE_SEND:
    /* Insert codes to transmit data */
    for(int i=0; i < arg_int;i++)
    {
      LL_SPI_TransmitData8(SPI2, *data++);
      while(!LL_SPI_IsActiveFlag_TXE(SPI2));
    }
    break;
  case U8X8_MSG_BYTE_INIT:
    /* Insert codes to begin SPI transmission */
    break;
  case U8X8_MSG_BYTE_SET_DC:
    /* Control DC pin, U8X8_MSG_GPIO_DC will be called */
    u8x8_gpio_SetDC(u8x8, arg_int);
    break;
  case U8X8_MSG_BYTE_START_TRANSFER:
    /* Select slave, U8X8_MSG_GPIO_CS will be called */
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_enable_level);
    LL_mDelay(1);
    break;
  case U8X8_MSG_BYTE_END_TRANSFER:
    LL_mDelay(1);
    /* Insert codes to end SPI transmission */
    u8x8_gpio_SetCS(u8x8, u8x8->display_info->chip_disable_level);
    break;
  default:
    return 0;
  }
  return 1;
}

uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
  static uint8_t buffer[32];
  static uint8_t buf_idx;
  uint8_t *data;

  switch(msg)
  {
  case U8X8_MSG_BYTE_SEND:
    data = (uint8_t *)arg_ptr;
    while( arg_int > 0 )
    {
      buffer[buf_idx++] = *data;
      data++;
      arg_int--;
    }
    break;
  case U8X8_MSG_BYTE_INIT:
    /* add your custom code to init i2c subsystem */
    break;
  case U8X8_MSG_BYTE_SET_DC:
    break;
  case U8X8_MSG_BYTE_START_TRANSFER:
    buf_idx = 0;
    break;
  case U8X8_MSG_BYTE_END_TRANSFER:
    I2C_MasterTx(DEVICE_ADDRESS, buffer, buf_idx);
    break;
  default:
    return 0;
  }
  return 1;
}
