#ifndef _25LC010A_H_
#define _25LC010A_H_

#include "main.h"

#define EEPROM_CMD_WRITE	0x02
#define EEPROM_CMD_READ		0x03
#define EEPROM_CMD_RDSR		0x05
#define EEPROM_CMD_WREN		0x06

#define EEPROM_PAGE_SIZE	16
#define EEPROM_TOTAL_SIZE	128

void EEPROM_Init(SPI_TypeDef* spi, GPIO_TypeDef* ss_port, uint16_t ss_pin);
void EEPROM_WriteEnable(void);
uint8_t EEPROM_ReadByte(uint8_t address);
void EEPROM_ReadBuffer(uint8_t address, uint8_t *buffer, uint8_t length);
void EEPROM_WriteByte(uint8_t address, uint8_t data);
void EEPROM_WriteBuffer(uint8_t address, uint8_t *buffer, uint8_t length);
void EEPROM_EraseAll(void);

#endif /* _25LC010A_H_ */
