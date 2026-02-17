#ifndef __FINGERPRINT_H
#define __FINGERPRINT_H

#include <stdint.h>

#include "stm32f1xx_hal.h"
#include "main.h"

#define GET_TOUCH HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin)

#define FINGERPRINT_OK 0
#define FINGERPRINT_FAIL 1
#define FINGERPRINT_NO_FINGER 2
#define FINGERPRINT_TIMEOUT 3

uint8_t Fingerprint_Identify(void);
uint8_t Fingerprint_Enroll(uint8_t id);
uint8_t Fingerprint_Delete(uint8_t id);
uint8_t Fingerprint_Empty(void);
uint8_t Fingerprint_GetImage(void);
uint8_t Fingerprint_GenChar(uint8_t buffer_id);


#endif