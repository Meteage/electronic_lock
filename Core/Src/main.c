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

#define STATE_IDLE 0 //默认状态
#define STATE_LOCKED 1 //锁定状态
#define STATE_OPEN 2 //开门状态

#define STATE_CHANGE_PWD 3//改密码状态
//属于 STATE_CHANGE_PWD
#define STATE_CHANGE_OLD 3 //修改输入旧密码
#define STATE_CHANGE_NEW1 4 //修改输入新密码1
#define STATE_CHANGE_NEW2 5 //修改输入新密码2
//STATE_CHANGE_FG
#define STATE_CHANGE_FG 6//修改指纹状态
#define STATE_CHANGE_FG_ADD 7//添加
#define STATE_CHANGE_FG_DEL 8//删除
#define STATE_CHANGE_FG_DEL_ALL 9//删除全部

#define LOCKED_TIME 10
#define LOCKED_COUNT 5

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
  char new_password[6+1] = {0}; //新密码

  char input_buffer[6+1] = {0}; //输入缓存
  char state_buffer[16] = {0}; //状态栏缓存
  char message_buffer[16] = {0};//提示信息缓存区

  sprintf(message_buffer, "Ready Input:");

  
  uint8_t index = 0;
  uint8_t error_count = 0; //错误次数

  uint32_t g_last_key_time = 0;
  uint8_t g_screen_off = 0;

  uint8_t g_state = STATE_CHANGE_FG_ADD; //一开始，先添加管理员指纹
  uint32_t g_lock_start = 0;
  uint8_t g_lock_seconds = 60;

  uint32_t g_open_start = 0;
  uint8_t g_open_seconds = 3;//秒

  //debug
  uint8_t X_Enable = 1; //是否开启掩码

  //延时等待器件初始化
  HAL_Delay(1000);

  //密码用AT24C02存储
  uint8_t check[7] = {0};
  AT24C02_ReadData(0x00, check, 7);
  if(check[0] == 0xFF) {  // EEPROM 默认值是 0xFF
      AT24C02_WritePage(0x00, (uint8_t *)&password, 7);  // 第一次使用才写默认密码
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    if(!g_screen_off){
      switch(g_state){
        case STATE_LOCKED:
          //计算并显示剩余时间 elapsed v.过去 adj.过去的
          uint8_t elapsed = (HAL_GetTick() - g_lock_start) / 1000;
          
          if (elapsed >= g_lock_seconds) {
            g_state = STATE_IDLE;
            error_count = 0;
            //输出提示信息
            sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
            sprintf(message_buffer, "Ready Input:");

            OLED_NewFrame();
            OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_ShowFrame();
          }
          else{
            uint8_t remain = g_lock_seconds - elapsed;
            //输出提示信息
            sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
            sprintf(message_buffer, "Lock %ds.",remain);

            OLED_NewFrame();
            OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
            OLED_ShowFrame();
          }
        break;
        case STATE_IDLE:
        {
          char display[6+1] = "______";//用于显示掩码
          for(int i=0;i<index;i++){
            if(index-1 == i){
              display[i] = input_buffer[i];//最后一位正常显示
            }
            else{
              display[i] = '*';
            }
          }

          //输出提示信息
          sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
          // sprintf(message_buffer, "Ready Input:");
          
          OLED_NewFrame();
          OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
          OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
          if(X_Enable){
            OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
          }
          else{
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
          }
          OLED_ShowFrame();
        }
        break;
        case STATE_OPEN:
      {    // 只在第一次进入时开门
      static uint8_t first_entry = 1;
      if(first_entry) {
          HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_SET);
          g_open_start = HAL_GetTick();  // 记录开门开始时间
          first_entry = 0;
      }
      
      // 计算已开门时间
      uint8_t elapsed = (HAL_GetTick() - g_open_start) / 1000;
      
      // 显示剩余时间
      sprintf(message_buffer, "Open %ds", g_open_seconds - elapsed);
      
      // 3秒后关门
      if(elapsed >= g_open_seconds) {
          // 关门
          HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(OPEN_LED_GPIO_Port, OPEN_LED_Pin, GPIO_PIN_SET);
          HAL_GPIO_WritePin(CLOSE_LED_GPIO_Port, CLOSE_LED_Pin, GPIO_PIN_RESET);
          
          // 清空输入
          memset(input_buffer, 0, 7);
          index = 0;
          
          // 返回空闲状态
          g_state = STATE_IDLE;
          sprintf(message_buffer, "Ready Input:");
          first_entry = 1;  // 重置标志，下次开门重新开门
      }
      
      // 显示界面（不重复开门）
      char display[6+1] = "______";
      for(int i=0;i<index;i++){
          if(index-1 == i) display[i] = input_buffer[i];
          else display[i] = '*';
      }
      
      OLED_NewFrame();
      OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
      OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
      OLED_ShowFrame();
      break;}
  
        case STATE_CHANGE_OLD:
        {
          char display[6+1] = "______";//用于显示掩码
          for(int i=0;i<index;i++){
            if(index-1 == i){
              display[i] = input_buffer[i];//最后一位正常显示
            }
            else{
              display[i] = '*';
            }
          }

          //输出提示信息
          sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
          sprintf(message_buffer, "Input OP:");
          
          OLED_NewFrame();
          OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
          OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
          if(X_Enable){
            OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
          }
          else{
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
          }
          OLED_ShowFrame();
        }
        break;
        case STATE_CHANGE_NEW1:
        {
          char display[6+1] = "______";//用于显示掩码
          for(int i=0;i<index;i++){
            if(index-1 == i){
              display[i] = input_buffer[i];//最后一位正常显示
            }
            else{
              display[i] = '*';
            }
          }

          //输出提示信息
          sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
          sprintf(message_buffer, "Input NP1:");
          
          OLED_NewFrame();
          OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
          OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
          if(X_Enable){
            OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
          }
          else{
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
          }
          OLED_ShowFrame();
        }
        break;
      case STATE_CHANGE_NEW2:
        {
          char display[6+1] = "______";//用于显示掩码
          for(int i=0;i<index;i++){
            if(index-1 == i){
              display[i] = input_buffer[i];//最后一位正常显示
            }
            else{
              display[i] = '*';
            }
          }

          //输出提示信息
          sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
          sprintf(message_buffer, "Input NP2:");
          
          OLED_NewFrame();
          OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
          OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
          if(X_Enable){
            OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
          }
          else{
            OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
          }
          OLED_ShowFrame();
        }
        break;
      }
    }
    
    
      // 扫描按键
      Key key = KEYBOARD20_Scan();
      //有按键或者有指纹
      if(key!=KEY_NULL || GET_TOUCH) {
          HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
          g_last_key_time = HAL_GetTick();  // 记录最后按键时间
          g_screen_off = 0;                 // 屏幕唤醒

          if(key == KEY_NULL){
            //响应指纹
            if(g_state == STATE_IDLE){
              uint8_t rsp = Fingerprint_Identify();
              if(rsp == FINGERPRINT_OK){
                //打印验证成功
                sprintf(message_buffer, "Identity Succeffuly");
                g_state = STATE_OPEN;
              }
              else {
                //打印验证失败
                sprintf(message_buffer, "Identity Fail");
              }
            }
            else if(g_state == STATE_CHANGE_FG){
              uint8_t rsp = Fingerprint_Identify();
              if(rsp == FINGERPRINT_OK){
                //打印验证成功
                g_state = STATE_CHANGE_FG_ADD;
              }else {
                //打印失败
                g_state = STATE_IDLE;
              }
            }
            else if(g_state == STATE_CHANGE_FG_ADD){
              uint8_t rsp = Fingerprint_Enroll(0x01);
              if(rsp == FINGERPRINT_OK){
                //打印成功
                g_state = STATE_IDLE;
              }
              else{
                //打印失败
              }

            }
            
          }
          else{
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
                if(g_state == STATE_IDLE){
                  // 清空输入
                  memset(input_buffer, 0, 7);
                  index = 0;
                  
                }
                else if(g_state == STATE_CHANGE_NEW1||g_state == STATE_CHANGE_NEW2||g_state == STATE_CHANGE_OLD){
                  g_state = STATE_IDLE;
                  // 清空输入
                  memset(input_buffer, 0, 7);
                  index = 0;
                }
              break;
              case KEY_ENT:
                //读取密码
                AT24C02_ReadData(0x00, password, 7);
                //确认键 
                if(g_state == STATE_IDLE){
                    //比较命名是否正确 用strcmp比较吧 那么就把存储的用字符串数组存起来
                    if(strcmp(password, input_buffer) == 0){
                      //清空error_count
                      error_count = 0;
                      g_state = STATE_OPEN;
                    }
                    else {
                      error_count++;//错误计算加1
                      if(error_count >= LOCKED_COUNT) {
                        //切换状态
                        g_state = STATE_LOCKED;
                        g_lock_start = HAL_GetTick();
                        g_lock_seconds = LOCKED_TIME;//10秒

                      }
                      else{
                        //输出提示信息
                        sprintf(state_buffer, "Mode:%d State:%c",g_state,'E');
                        sprintf(message_buffer, "Error!");

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
                        OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
                        OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
                        if(X_Enable){
                          OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
                        }
                        else{
                          OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
                        }
                        OLED_ShowFrame();
                        //显示3秒后关闭
                        HAL_Delay(3000);

                        //重置
                        // 清空输入
                        memset(input_buffer, 0, 7);
                        index = 0;

                        //输出提示信息
                        sprintf(state_buffer, "Mode:%d State:%c",g_state,'R');
                        sprintf(message_buffer, "Ready Input:");
                        
                        OLED_NewFrame();
                        OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
                        OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
                        if(X_Enable){
                          OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
                        }
                        else{
                          OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
                        }
                        OLED_ShowFrame();
                      }
                      
                    }
                }
                else if(g_state == STATE_CHANGE_OLD){
                  //比较输入与旧密码
                  uint8_t result = strcmp(password, input_buffer);
                  if(result == 0){
                    //成功进入下一状态
                    g_state =STATE_CHANGE_NEW1;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                  else{
                    //失败
                    g_state = STATE_IDLE;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                }
                else if (g_state == STATE_CHANGE_NEW1) {
                  //复制
                  if(strlen(input_buffer)==6){
                    strcpy(new_password, input_buffer);
                    g_state = STATE_CHANGE_NEW2;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                  else{
                    //失败
                    g_state = STATE_IDLE;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                }
                else if (g_state == STATE_CHANGE_NEW2) {
                  //复制
                  if(strcmp(new_password,input_buffer)==0){
                    //设置新密码
                    //strcpy(password, input_buffer);
                    AT24C02_WritePage(0x00, input_buffer, 7);
                    //改变
                    g_state = STATE_IDLE;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                  else{
                    //失败
                    g_state = STATE_IDLE;
                    // 清空输入
                    memset(input_buffer, 0, 7);
                    index = 0;
                  }
                }
                
              break;
              case KEY_F1:
              {
                //切换到修改模式
                g_state = STATE_CHANGE_OLD;

                // 清空输入
                memset(input_buffer, 0, 7);
                index = 0;

              }
              break;
              case KEY_F2:
              {
                //切换到修改指纹模式
                
              }
              break;
            }
          }
          
          
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
