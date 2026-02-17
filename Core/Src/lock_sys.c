// #include "lock_sys.h"
// #include "main.h"
// #include "ssd1306/font.h"
// #include "ssd1306/oled.h"


// char input_buffer[6+1] = {0}; //输入缓存
// char state_buffer[16] = {0}; //状态栏缓存
// char message_buffer[16] = {0};//提示信息缓存区

// //显示函数
// void LOCK_DiplayMessage(uint8_t mode,uint8_t state,uint8_t * message){
//     char display[6+1] = "______";//用于显示掩码
//     for(int i=0;i<index;i++){
//         if(index-1 == i){
//         display[i] = input_buffer[i];//最后一位正常显示
//         }
//         else{
//         display[i] = '*';
//         }
//     }
//     //输出提示信息
//     sprintf(state_buffer, "Mode:%d State:%c",(char)mode,(char)state);
    
//     OLED_NewFrame();
//     OLED_PrintString(0, 0, state_buffer, &font16x16, OLED_COLOR_NORMAL);
//     OLED_PrintString(0, 1*16, message_buffer, &font16x16, OLED_COLOR_NORMAL);
//     if(X_Enable){
//     OLED_PrintString(0, 2*16, display, &font16x16, OLED_COLOR_NORMAL);
//     }
//     else{
//     OLED_PrintString(0, 2*16, input_buffer, &font16x16, OLED_COLOR_NORMAL);
//     }
//     OLED_ShowFrame();
// }




          