#ifndef __AT24C02_H__
#define __AT24C02_H__

#include <stdint.h>

uint8_t AT24C02_IsBusy(void);
uint8_t AT24C02_WriteByte(uint8_t byte_address, uint8_t data);
uint8_t AT24C02_WritePage(uint8_t byte_address, uint8_t *data, uint8_t len);

uint8_t AT24C02_ReadByte(uint8_t data_address,uint8_t *byte);
uint8_t AT24C02_ReadData(uint8_t data_address,uint8_t *data,uint8_t len);

#endif