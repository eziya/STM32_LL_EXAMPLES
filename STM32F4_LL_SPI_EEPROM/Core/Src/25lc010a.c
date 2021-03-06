#include "25lc010a.h"

SPI_TypeDef* EEPROM_SPI;
GPIO_TypeDef* EEPROM_SS_Port;
uint16_t EEPROM_SS_Pin;

static void SS_Select();
static void SS_Deselect();
static uint8_t SPI_TxRx(uint8_t data);

void EEPROM_Init(SPI_TypeDef* spi, GPIO_TypeDef* ss_port, uint16_t ss_pin)
{
  // configure SPI pins
  EEPROM_SPI = spi;
  EEPROM_SS_Port = ss_port;
  EEPROM_SS_Pin = ss_pin;

  // enable SPI
  LL_SPI_Enable(EEPROM_SPI);

  // eeprom deselect
  SS_Deselect();
}

void EEPROM_WriteEnable(void)
{
  SS_Select();
  SPI_TxRx(EEPROM_CMD_WREN);
  SS_Deselect();
}

uint8_t EEPROM_ReadByte(uint8_t address)
{
  uint8_t ret;

  SS_Select();
  SPI_TxRx(EEPROM_CMD_READ);
  SPI_TxRx(address);
  // transmit garbage byte to receive
  ret = SPI_TxRx(0x0);
  SS_Deselect();

  return ret;
}

void EEPROM_ReadBuffer(uint8_t address, uint8_t *buffer, uint8_t length)
{
  // read multiple bytes
  for(uint8_t i=0; i < length; i++)
  {
    buffer[i] = EEPROM_ReadByte(address+i);
  }
}

void EEPROM_WriteByte(uint8_t address, uint8_t data)
{
  EEPROM_WriteEnable();
  SS_Select();
  SPI_TxRx(EEPROM_CMD_WRITE);
  SPI_TxRx(address);
  SPI_TxRx(data);
  SS_Deselect();

  // wait write cycle time 5ms
  LL_mDelay(5);
}

void EEPROM_WriteBuffer(uint8_t address, uint8_t *buffer, uint8_t length)
{
  EEPROM_WriteEnable();

  SS_Select();
  SPI_TxRx(EEPROM_CMD_WRITE);
  SPI_TxRx(address);

  for(uint8_t i = 0; i < length; i++)
  {
    // when page is changed while writing, you need to send command & address again.
    if( (address+i) % EEPROM_PAGE_SIZE == 0 && i != 0 )
    {
      SS_Deselect();
      LL_mDelay(5);

      // write enable
      EEPROM_WriteEnable();
      SS_Select();
      SPI_TxRx(EEPROM_CMD_WRITE);
      SPI_TxRx(address+i);
    }
    SPI_TxRx(buffer[i]);
  }

  SS_Deselect();

  // wait write cycle time 5ms
  LL_mDelay(5);
}

void EEPROM_EraseAll(void)
{
  uint8_t address = 0x0;

  EEPROM_WriteEnable();

  while(address < EEPROM_TOTAL_SIZE)
  {
    SS_Select();

    SPI_TxRx(EEPROM_CMD_WRITE);
    SPI_TxRx(address);

    // send data(0x0) EEPROM_PAGE_SIZE times
    for(uint8_t i=0; i < EEPROM_PAGE_SIZE; i++)
    {
      SPI_TxRx(0x0);
    }

    SS_Deselect();

    // wait write cycle time 5ms
    LL_mDelay(5);
    address += EEPROM_PAGE_SIZE;
  }
}

static void SS_Select()
{
  LL_GPIO_ResetOutputPin(EEPROM_SS_Port, EEPROM_SS_Pin);
  LL_mDelay(1);
}

static void SS_Deselect()
{
  LL_GPIO_SetOutputPin(EEPROM_SS_Port, EEPROM_SS_Pin);
  LL_mDelay(1);
}

static uint8_t SPI_TxRx(uint8_t data)
{
  // transmit
  LL_SPI_TransmitData8(EEPROM_SPI, data);
  while(!LL_SPI_IsActiveFlag_TXE(EEPROM_SPI));

  // receive
  while(!LL_SPI_IsActiveFlag_RXNE(EEPROM_SPI));
  return LL_SPI_ReceiveData8(EEPROM_SPI);
}

