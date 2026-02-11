#ifndef __SOFTWARE_I2C_H__
#define __SOFTWARE_I2C_H__

#include "main.h"
#include "stm32f1xx_hal_gpio.h"

#define SCL_W(state) HAL_GPIO_WritePin(OLED_SCK_GPIO_Port, OLED_SCK_Pin , (GPIO_PinState) (state))
#define SDA_W(state) HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin , (GPIO_PinState) (state))

#define SDA_R HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin)


void SOFTWARE_I2C_Init(void);

void SOFTWARE_I2C_Start(void);
void SOFTWARE_I2C_Stop(void);

//发送一个字节 8bit 高位先发
void SOFTWARE_I2C_SendByte(uint8_t byte);

uint8_t SOFTWARE_I2C_ReceiveByte();

void SOFTWARE_I2C_SendAck(uint8_t ack);
uint8_t SOFTWARE_I2C_ReceiveAck(void);

uint8_t SOFTWARE_I2C_WriteToDevice(uint8_t devAddr, uint8_t *data, uint8_t len);

#endif