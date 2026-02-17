#ifndef __FRINGERPRINT_H__
#define __FRINGERPRINT_H__

#include "stm32f1xx_hal.h"
#include "main.h"

#define GET_TOUCH HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin)

// 指纹状态定义
#define FINGERPRINT_OK      0
#define FINGERPRINT_FAIL    1
#define FINGERPRINT_NO_FINGER 2

uint8_t Fingerprint_GetChar(uint8_t buffer_id);
uint8_t Fingerprint_Enroll(uint8_t page_id);
uint8_t Fingerprint_Identify(void);

#endif