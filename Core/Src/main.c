/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "keyboard20.h"
#include <stdint.h>
#include <stdio.h>
#include "software_i2c.h"
#include "ssd1306/oled.h"
#include "lock_sys.h"
#include <string.h>

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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  SOFTWARE_I2C_Init();
  OLED_Init();
  KEYBOARD20_Init();
  HAL_UART_Transmit(&huart1, (const uint8_t *)"TEST", 5, HAL_MAX_DELAY);

  char password[6+1] = "1234#*"; //正确密码
  char input_buffer[6+1] = {0}; //输入缓存
  uint8_t index = 0;
  uint8_t error_count = 0; //错误次数

  uint32_t g_last_key_time = 0;
  uint8_t g_screen_off = 0;

  //debug
  uint8_t X_Enable = 1; //是否开启掩码

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
   // 扫描按键
    Key key = KEYBOARD20_Scan();
    if(key) {
        HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
        g_last_key_time = HAL_GetTick();  // 记录最后按键时间
        g_screen_off = 0;                 // 屏幕唤醒

        //响应按键
        switch (key) {
          case KEY_J:
          case KEY_X:
          case KEY_0...KEY_9:
            //输入
            if(index<6){
              input_buffer[index] = KEYBOARD20_GetKeyValue(key);
              index++;
            }
          break;
          case KEY_LEFT:
            //删除一位
            if(index>=1){
              //至少有一位才能删
              index--;
              input_buffer[index] = 0;
            }
          break;
          case KEY_ESC:
            // 清空输入
            memset(input_buffer, 0, 7);
            index = 0;
          break;
          case KEY_ENT:
            //确认键 
            //比较命名是否正确 用strcmp比较吧 那么就把存储的用字符串数组存起来
            if(strcmp(password, input_buffer) == 0){
              //清空error_count
              error_count = 0;
              OLED_NewFrame();
              OLED_PrintString(1, 1, "Right", &font16x16, OLED_COLOR_NORMAL);
              OLED_ShowFrame();

              //开门3秒
              HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_SET);
              HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_RESET);
              HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_SET);

              HAL_Delay(3000);
              //关门
              HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_RESET);
              HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_SET);
              HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_RESET);

              //重置
              // 清空输入
              memset(input_buffer, 0, 7);
              index = 0;
            }
            else {
              error_count++;//错误计算加1
              if(error_count >= 5) {
                OLED_NewFrame();
                OLED_PrintString(1, 1, "Lock 10s", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
                HAL_Delay(10*1000);
                error_count = 0;
                OLED_NewFrame();
                OLED_PrintString(1, 1, "Ready", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
              }
              else{
                OLED_NewFrame();
                OLED_PrintString(1, 1, "Error", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
                //显示3秒后关闭
                HAL_Delay(3000);
                //重置
                // 清空输入
                memset(input_buffer, 0, 7);
                index = 0;
              }
              
            }
          break;
        }
        char display[6+1] = "______";//用于显示掩码
        for(int i=0;i<index;i++){
          if(index-1 == i){
            display[i] = input_buffer[i];//最后一位正常显示
          }
          else{
            display[i] = '*';
          }
        }

        OLED_NewFrame();
        if(X_Enable){
          OLED_PrintString(1, 1, (char *)display, &font16x16, OLED_COLOR_NORMAL);
        }else{
          OLED_PrintString(1, 1, (char *)input_buffer, &font16x16, OLED_COLOR_NORMAL);
        }
        OLED_ShowFrame();
    }

    // 检查空闲超时
    if(!g_screen_off && (HAL_GetTick() - g_last_key_time > 15000)) {
        // 1. 清空输入
        memset(input_buffer, 0, 7);
        index = 0;
        
        // 2. 关屏幕（黑屏）
        OLED_NewFrame();      // 清空显存
        OLED_ShowFrame();     // 显示黑屏
        
        g_screen_off = 1;    // 标记已息屏
    }

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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
#ifdef USE_FULL_ASSERT
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
