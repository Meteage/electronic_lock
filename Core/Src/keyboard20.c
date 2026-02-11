#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "keyboard20.h"

 //列写行读
#define COL0_W(state) HAL_GPIO_WritePin(PIN_1_GPIO_Port, PIN_1_Pin, (GPIO_PinState)(state))
#define COL1_W(state) HAL_GPIO_WritePin(PIN_2_GPIO_Port, PIN_2_Pin, (GPIO_PinState)(state))
#define COL2_W(state) HAL_GPIO_WritePin(PIN_3_GPIO_Port, PIN_3_Pin, (GPIO_PinState)(state))
#define COL3_W(state) HAL_GPIO_WritePin(PIN_4_GPIO_Port, PIN_4_Pin, (GPIO_PinState)(state))

#define ROW0_R HAL_GPIO_ReadPin(PIN_9_GPIO_Port, PIN_9_Pin)
#define ROW1_R HAL_GPIO_ReadPin(PIN_8_GPIO_Port, PIN_8_Pin)
#define ROW2_R HAL_GPIO_ReadPin(PIN_7_GPIO_Port, PIN_7_Pin)
#define ROW3_R HAL_GPIO_ReadPin(PIN_6_GPIO_Port, PIN_6_Pin)
#define ROW4_R HAL_GPIO_ReadPin(PIN_5_GPIO_Port, PIN_5_Pin)

void KEYBOARD_Init(void){
    COL0_W(1);COL1_W(1);COL2_W(1);COL3_W(1);
}

Key KEYBOARD20_Scan(void){
    //1000
    COL0_W(1);COL1_W(0);COL2_W(0);COL3_W(0);
    HAL_Delay(1);  // 电平稳定
    if(ROW0_R == 1){
        HAL_Delay(20);
        while(ROW0_R == 1);
        return KEY_F1;
    }
    else if (ROW1_R == 1) {
        HAL_Delay(20);
        while(ROW1_R == 1);
        return KEY_1;
    }
    else if (ROW2_R == 1) {
        HAL_Delay(20);
        while(ROW2_R == 1);
        return KEY_4;
    }
    else if (ROW3_R == 1) {
        HAL_Delay(20);
        while(ROW3_R == 1);
        return KEY_7;
    }
    else if (ROW4_R == 1) {
        HAL_Delay(20);
        while(ROW4_R == 1);
        return KEY_LEFT;
    }

    //0100
    COL0_W(0);COL1_W(1);COL2_W(0);COL3_W(0);
    HAL_Delay(1);  // 电平稳定
    if(ROW0_R == 1){
        HAL_Delay(20);
        while(ROW0_R == 1);
        return KEY_F2;
    }
    else if (ROW1_R == 1) {
        HAL_Delay(20);
        while(ROW1_R == 1);
        return KEY_2;
    }
    else if (ROW2_R == 1) {
        HAL_Delay(20);
        while(ROW2_R == 1);
        return KEY_5;
    }
    else if (ROW3_R == 1) {
        HAL_Delay(20);
        while(ROW3_R == 1);
        return KEY_8;
    }
    else if (ROW4_R == 1) {
        HAL_Delay(20);
        while(ROW4_R == 1);
        return KEY_0;
    }

    //0010
    COL0_W(0);COL1_W(0);COL2_W(1);COL3_W(0);
    HAL_Delay(1);  // 电平稳定
    if(ROW0_R == 1){
        HAL_Delay(20);
        while(ROW0_R == 1);
        return KEY_J;
    }
    else if (ROW1_R == 1) {
        HAL_Delay(20);
        while(ROW1_R == 1);
        return KEY_3;
    }
    else if (ROW2_R == 1) {
        HAL_Delay(20);
        while(ROW2_R == 1);
        return KEY_6;
    }
    else if (ROW3_R == 1) {
        HAL_Delay(20);
        while(ROW3_R == 1);
        return KEY_9;
    }
    else if (ROW4_R == 1) {
        HAL_Delay(20);
        while(ROW4_R == 1);
        return KEY_RIGHT;
    }

    //0001
    COL0_W(0);COL1_W(0);COL2_W(0);COL3_W(1);
    HAL_Delay(1);  // 电平稳定
    if(ROW0_R == 1){
        HAL_Delay(20);
        while(ROW0_R == 1);
        return KEY_X;
    }
    else if (ROW1_R == 1) {
        HAL_Delay(20);
        while(ROW1_R == 1);
        return KEY_UP;
    }
    else if (ROW2_R == 1) {
        HAL_Delay(20);
        while(ROW2_R == 1);
        return KEY_DOWN;
    }
    else if (ROW3_R == 1) {
        HAL_Delay(20);
        while(ROW3_R == 1);
        return KEY_ESC;
    }
    else if (ROW4_R == 1) {
        HAL_Delay(20);
        while(ROW4_R == 1);
        return KEY_ENT;
    }

    return KEY_NULL;
}


