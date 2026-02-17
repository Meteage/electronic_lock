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
#include "at24c02.h"
#include "fingerprint.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// ==================== 状态定义 ====================
#define STATE_IDLE               0  // 空闲
#define STATE_LOCKED             1  // 锁定
#define STATE_OPEN               2  // 开门

#define STATE_CHANGE_OLD         3  // 改密码-输入旧密码
#define STATE_CHANGE_NEW1        4  // 改密码-输入新密码1
#define STATE_CHANGE_NEW2        5  // 改密码-输入新密码2

#define STATE_FINGER_AUTH        6  // 指纹管理前验证管理员
#define STATE_FINGER_MENU        7  // 指纹菜单
#define STATE_FINGER_ADD         8  // 添加指纹
#define STATE_FINGER_DEL         9  // 删除指纹
#define STATE_FINGER_DEL_ALL     10  // 清空所有指纹
#define STATE_FINGER_LIST       11  // 查看指纹列表

// ==================== 常量定义 ====================
#define LOCKED_TIME   10  // 锁定时间(秒)
#define LOCKED_COUNT   5  // 最大错误次数
#define OPEN_TIME      3  // 开门时间(秒)
#define SCREEN_TIMEOUT 15 // 屏幕超时(秒)

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
char password[7] = "1234#*";      // 正确密码
char new_password[7] = {0};       // 新密码临时存储
char input_buffer[7] = {0};       // 输入缓冲区
char state_buffer[16] = {0};      // 状态栏
char message_buffer[16] = "Ready Input:";  // 提示信息

int my_index = 0;                 // 当前输入位数
uint8_t error_count = 0;           // 错误次数

uint32_t g_last_key_time = 0;      // 上次按键时间
uint8_t g_screen_off = 0;          // 屏幕是否关闭

uint8_t g_state = STATE_FINGER_ADD;      // 当前状态

uint32_t g_lock_start = 0;          // 锁定开始时间
uint32_t g_open_start = 0;          // 开门开始时间

// 标志位
uint8_t g_open_done = 0;            // 是否已执行开门动作
uint8_t g_enroll_step = 0;          // 指纹录入步骤
uint8_t X_Enable = 1;                // 调试用

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void OpenDoor(void);
void CloseDoor(void);
void UpdateDisplay(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// ==================== 硬件控制函数 ====================
void OpenDoor(void) {
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_SET);
}

void CloseDoor(void) {
    HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_RESET);
}

// ==================== 显示更新函数 ====================
void UpdateDisplay(void) {
    if(g_screen_off) return;
    
    char display[7] = "______";
    for(int i=0; i<my_index; i++) {
        if(i == my_index-1) display[i] = input_buffer[i];
        else display[i] = '*';
    }
    
    sprintf(state_buffer, "Mode:%d", g_state);
    
    OLED_NewFrame();
    OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
    
    if(g_state == STATE_IDLE || g_state == STATE_CHANGE_OLD || 
       g_state == STATE_CHANGE_NEW1 || g_state == STATE_CHANGE_NEW2) {
        OLED_PrintString(0, 32, display, &font16x16, OLED_COLOR_NORMAL);
    }
    OLED_ShowFrame();
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
  
  HAL_Delay(1000);
  
  // 从EEPROM读取密码
  uint8_t check[7] = {0};
  AT24C02_ReadData(0x00, check, 7);
  if(check[0] == 0x00) {
      AT24C02_WritePage(0x00, (uint8_t*)password, 7);
  } else {
      memcpy(password, check, 7);
  }
  
  // 开机显示
  sprintf(message_buffer, "Ready Input:");
  UpdateDisplay();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
     // ==================== 状态机处理 ====================
        switch(g_state) {
            case STATE_LOCKED: {
                uint32_t elapsed = (HAL_GetTick() - g_lock_start) / 1000;
                if(elapsed >= LOCKED_TIME) {
                    g_state = STATE_IDLE;
                    error_count = 0;
                    sprintf(message_buffer, "Ready Input:");
                } else {
                    sprintf(message_buffer, "Lock %lds", LOCKED_TIME - elapsed);
                }
                UpdateDisplay();
                break;
            }
            
            case STATE_OPEN: {
                if(!g_open_done) {
                    OpenDoor();
                    g_open_start = HAL_GetTick();
                    g_open_done = 1;
                }
                
                uint32_t elapsed = (HAL_GetTick() - g_open_start) / 1000;
                sprintf(message_buffer, "Open %lds", OPEN_TIME - elapsed);
                UpdateDisplay();
                
                if(elapsed >= OPEN_TIME) {
                    CloseDoor();
                    memset(input_buffer, 0, 7);
                    my_index = 0;
                    g_state = STATE_IDLE;
                    g_open_done = 0;
                    sprintf(message_buffer, "Ready Input:");
                }
                break;
            }
            
            case STATE_FINGER_MENU: {
                static uint8_t first = 1;
                if(first) {
                    strcpy(message_buffer, "1:Add 2:Del");
                    first = 0;
                }
                UpdateDisplay();
                break;
            }
            
            case STATE_FINGER_ADD: {
                sprintf(message_buffer, "Put finger %d/2", g_enroll_step + 1);
                UpdateDisplay();
                break;
            }
            
            case STATE_FINGER_DEL: {
                char display[7] = "______";
                for(int i=0; i<my_index; i++) display[i] = input_buffer[i];
                OLED_NewFrame();
                OLED_PrintString(0, 0, "Del Finger ID", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 16, display, &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 32, "0-9 to input", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
                break;
            }
            
            case STATE_FINGER_DEL_ALL: {
                OLED_NewFrame();
                OLED_PrintString(0, 0, "Delete ALL?", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 16, "ENT=Yes ESC=No", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
                break;
            }

            case STATE_FINGER_AUTH: {
                char display[7] = "______";
                for(int i=0; i<my_index; i++) {
                    if(i == my_index-1) display[i] = input_buffer[i];
                    else display[i] = '*';
                }
                
                OLED_NewFrame();
                OLED_PrintString(0, 0, "Admin Auth", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 16, "Input PWD:", &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 32, display, &font16x16, OLED_COLOR_NORMAL);
                OLED_PrintString(0, 48, "OR put finger", &font16x16, OLED_COLOR_NORMAL);
                OLED_ShowFrame();
                break;
            }
            
            default:
                UpdateDisplay();
                break;
        }
        
        // ==================== 按键/指纹扫描 ====================
        Key key = KEYBOARD20_Scan();
        uint8_t touch = HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin);
        
        if(key != KEY_NULL || touch) {
            g_last_key_time = HAL_GetTick();
            g_screen_off = 0;
            
            // ======== 指纹处理 ========
            if(key == KEY_NULL && touch) {
                
                switch(g_state) {
                    case STATE_IDLE: {
                        uint8_t rsp = Fingerprint_Identify();
                        if(rsp == FINGERPRINT_OK) {
                            sprintf(message_buffer, "Welcome!");
                            g_state = STATE_OPEN;
                        } else {
                            sprintf(message_buffer, "Try Again");
                        }
                        break;
                    }
                    case STATE_FINGER_AUTH: {
                        uint8_t rsp = Fingerprint_Identify();
                        if(rsp == FINGERPRINT_OK) {
                            // 指纹验证成功，进入指纹菜单
                            g_state = STATE_FINGER_MENU;
                            sprintf(message_buffer, "1:Add 2:Del");
                        } else {
                            sprintf(message_buffer, "Auth Fail");
                            g_state = STATE_IDLE;
                        }
                        break;
                    }
                    case STATE_FINGER_ADD: {
                        uint8_t rsp = Fingerprint_Enroll(1);  // 用ID=1
                        g_enroll_step++;
                        if(g_enroll_step >= 3) {
                          sprintf(message_buffer, "Failed");
                            g_state = STATE_IDLE;
                            g_enroll_step = 0;
                           
                        }else if(rsp == FINGERPRINT_OK) {
                            sprintf(message_buffer, "Add OK!");
                            g_state = STATE_IDLE;
                            g_enroll_step = 0;
                        }
                        break;
                    }
                }
            }
            
            // ======== 按键处理 ========
            else {
                switch(key) {
                    // ===== 数字键 =====
                    case KEY_0: case KEY_1: case KEY_2: case KEY_3: case KEY_4:
                    case KEY_5: case KEY_6: case KEY_7: case KEY_8: case KEY_9:
                    case KEY_J: case KEY_X:
                        if(g_state == STATE_FINGER_MENU) {
                            switch(key) {
                                case KEY_1: g_state = STATE_FINGER_ADD; g_enroll_step = 0; break;
                                case KEY_2: g_state = STATE_FINGER_DEL; memset(input_buffer,0,7); my_index=0; break;
                                case KEY_3: g_state = STATE_FINGER_DEL_ALL; break;
                            }
                        }
                        else if(g_state == STATE_FINGER_DEL) {
                            if(my_index < 2) {
                                input_buffer[my_index] = KEYBOARD20_GetKeyValue(key);
                                my_index++;
                            }
                        }
                        else {
                            if(my_index < 6) {
                                input_buffer[my_index] = KEYBOARD20_GetKeyValue(key);
                                my_index++;
                            }
                        }
                        break;
                    
                    // ===== 删除键 =====
                    case KEY_LEFT:
                        if(my_index > 0) {
                            my_index--;
                            input_buffer[my_index] = 0;
                        }
                        break;
                    
                    // ===== 清空键 =====
                    case KEY_ESC:
                        if(g_state >= STATE_FINGER_MENU) {
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                            sprintf(message_buffer, "Ready Input:");
                        } else if(g_state == STATE_IDLE) {
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                        } else if(g_state == STATE_CHANGE_NEW1 || 
                                  g_state == STATE_CHANGE_NEW2 || 
                                  g_state == STATE_CHANGE_OLD) {
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                        }else if(g_state == STATE_FINGER_AUTH) {
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                            sprintf(message_buffer, "Ready Input:");
                        }
                        break;
                    
                    // ===== 确认键 =====
                    case KEY_ENT:
                        AT24C02_ReadData(0x00, password, 7);
                        
                        if(g_state == STATE_IDLE) {
                            if(strcmp(password, input_buffer) == 0) {
                                error_count = 0;
                                g_state = STATE_OPEN;
                            } else {
                                error_count++;
                                if(error_count >= LOCKED_COUNT) {
                                    g_state = STATE_LOCKED;
                                    g_lock_start = HAL_GetTick();
                                } else {
                                    sprintf(message_buffer, "Error %d/%d", error_count, LOCKED_COUNT);
                                    memset(input_buffer, 0, 7);
                                    my_index = 0;
                                }
                            }
                        }
                        else if(g_state == STATE_CHANGE_OLD) {
                            if(strcmp(password, input_buffer) == 0) {
                                g_state = STATE_CHANGE_NEW1;
                                memset(input_buffer, 0, 7);
                                my_index = 0;
                                sprintf(message_buffer, "New PWD:");
                            } else {
                                g_state = STATE_IDLE;
                                memset(input_buffer, 0, 7);
                                my_index = 0;
                                sprintf(message_buffer, "Ready Input:");
                            }
                        }
                        else if(g_state == STATE_CHANGE_NEW1) {
                            if(my_index == 6) {
                                strcpy(new_password, input_buffer);
                                g_state = STATE_CHANGE_NEW2;
                                memset(input_buffer, 0, 7);
                                my_index = 0;
                                sprintf(message_buffer, "Confirm:");
                            }
                        }
                        else if(g_state == STATE_CHANGE_NEW2) {
                            if(strcmp(new_password, input_buffer) == 0) {
                                AT24C02_WritePage(0x00, input_buffer, 7);
                                sprintf(message_buffer, "Change OK");
                            } else {
                                sprintf(message_buffer, "Not Match");
                            }
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                            HAL_Delay(1000);
                            sprintf(message_buffer, "Ready Input:");
                        }
                        else if(g_state == STATE_FINGER_DEL) {
                            uint8_t id = 0;
                            for(uint8_t i=0; i<my_index; i++) {
                                id = id*10 + (input_buffer[i] - '0');
                            }
                            uint8_t rsp = Fingerprint_Delete(id);
                            if(rsp == FINGERPRINT_OK) {
                                sprintf(message_buffer, "Del OK");
                            } else {
                                sprintf(message_buffer, "Del Failed");
                            }
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                        }
                        else if(g_state == STATE_FINGER_DEL_ALL) {
                            Fingerprint_Empty();
                            sprintf(message_buffer, "All Deleted");
                            g_state = STATE_IDLE;
                            memset(input_buffer, 0, 7);
                            my_index = 0;
                        }
                        else if(g_state == STATE_FINGER_AUTH) {
                            if(strcmp(password, input_buffer) == 0) {
                                // 密码验证成功，进入指纹菜单
                                g_state = STATE_FINGER_MENU;
                                memset(input_buffer, 0, 7);
                                my_index = 0;
                                sprintf(message_buffer, "1:Add 2:Del");
                            } else {
                                // 密码错误
                                sprintf(message_buffer, "Auth Fail");
                                g_state = STATE_IDLE;
                                memset(input_buffer, 0, 7);
                                my_index = 0;
                                HAL_Delay(1000);
                                sprintf(message_buffer, "Ready Input:");
                            }
                        }
                        break;
                    
                    // ===== 功能键 =====
                    case KEY_F1:
                        g_state = STATE_CHANGE_OLD;
                        memset(input_buffer, 0, 7);
                        my_index = 0;
                        sprintf(message_buffer, "Old PWD:");
                        break;
                    
                    case KEY_F2:{
                        g_state = STATE_FINGER_AUTH;  // 先进入验证模式
                        memset(input_buffer, 0, 7);
                        my_index = 0;
                        sprintf(message_buffer, "Admin Auth:");
                        break;
                        }
                    break;
                        
                }
            }
        }
        
        // ==================== 空闲超时 ====================
        if(!g_screen_off && (HAL_GetTick() - g_last_key_time > SCREEN_TIMEOUT * 1000)) {
            memset(input_buffer, 0, 7);
            my_index = 0;
            OLED_NewFrame();
            OLED_ShowFrame();
            g_screen_off = 1;
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
