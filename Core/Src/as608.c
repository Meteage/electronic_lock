#include "as608.h"
#include "usart.h"
#include "string.h"
#include <stdint.h>

#define AS608_UART huart1
#define AS608_ADDRESS 0xFFFFFFFF  // 默认地址

uint8_t rx_buffer[20];
uint8_t rx_count = 12;
uint8_t rx_flag = 0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &AS608_UART) {
        // 处理收到的数据
        rx_flag = 1;
    }
}

/* 底层：字节收发 */
void AS608_UART_IT_Send(uint8_t *data, uint16_t len) {
    HAL_UART_Transmit_IT(&AS608_UART, data, len);
}

void AS608_UART_IT_Receive(uint8_t *buf, uint16_t len) {
    HAL_UART_Receive_IT(&AS608_UART, buf, len);
}


// void AS608_WriteCmd(uint8_t cmd, uint8_t *params, uint8_t param_len) {
//     uint8_t cmd_buffer[20];
//     uint16_t index = 0;
    
//     // 包头
//     cmd_buffer[index++] = 0xEF;
//     cmd_buffer[index++] = 0x01;
    
//     // 地址 (4字节)
//     cmd_buffer[index++] = (AS608_ADDRESS >> 24) & 0xFF;
//     cmd_buffer[index++] = (AS608_ADDRESS >> 16) & 0xFF;
//     cmd_buffer[index++] = (AS608_ADDRESS >> 8) & 0xFF;
//     cmd_buffer[index++] = AS608_ADDRESS & 0xFF;
    
//     // 包标识 (命令包)
//     cmd_buffer[index++] = 0x01;
    
//     // 包长度 = 1(指令码) + param_len +1 校验核
//     uint16_t pkt_len = 2 + param_len;
//     cmd_buffer[index++] = (pkt_len >> 8) & 0xFF;
//     cmd_buffer[index++] = pkt_len & 0xFF;
    
//     // 指令码
//     cmd_buffer[index++] = cmd;
    
//     // 参数
//     for(uint8_t i = 0; i < param_len; i++) {
//         cmd_buffer[index++] = params[i];
//     }
    
//     // 计算校验和 (从包标识到参数末尾)
//     uint16_t sum = 0;
//     for(uint8_t i = 6; i < index; i++) {  // 从包标识开始
//         sum += cmd_buffer[i];
//     }
//     cmd_buffer[index++] = (sum >> 8) & 0xFF;
//     cmd_buffer[index++] = sum & 0xFF;
    
//     // 发送
//     AS608_UART_IT_Send(cmd_buffer, index);
// }


uint8_t AS608_Transceive(uint8_t *tx, uint16_t tx_len, uint8_t *rx, uint16_t rx_len) {
    // 清标志
    rx_flag = 0;
    rx_count = rx_len;
    
    // 清空缓冲区（关键！）
    memset(rx_buffer, 0, sizeof(rx_buffer));
    
    // 先开接收
    HAL_UART_Receive_IT(&AS608_UART, rx_buffer, rx_len);
    
    // 再发送
    HAL_UART_Transmit(&AS608_UART, tx, tx_len,100);
    
    // 等待接收完成
    uint32_t start = HAL_GetTick();
    while(!rx_flag && (HAL_GetTick() - start < 500));
    
    if(!rx_flag) {
        return 0;
    }
    
    // 只复制有效数据
    memcpy(rx, rx_buffer, rx_len);
    rx_flag = 0;
    
    return 1;
}

//返回确认码
uint8_t AS608_GetImage(void){

    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_getimage[] = {
        0xEF, 0x01,
        0xFF, 0xFF, 0xFF, 0xFF,
        0x01,
        0x00, 0x03,
        0x01,
        0x00, 0x05
    };

    AS608_Transceive(cmd_getimage,12, rsp_buffer, 12);

    return rsp_buffer[9];
}

//返回确认码
uint8_t AS608_GenChar(uint8_t buffer_id){

    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x04,//长度
        0x02,//指令码
        buffer_id,//缓存区
        0x00, 0x00//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}

//返回确认码
uint8_t AS608_Match(){

    uint8_t rsp_buffer[14] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x03,//长度
        0x03,//指令码
        0x00, 0x07//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}


//返回确认码
uint8_t AS608_RegModel(){

    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x03,//长度
        0x05,//指令码
        0x00, 0x09//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}

//返回确认码
uint8_t AS608_StoreChar(uint8_t buffer_id,uint8_t page_id){

    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x06,//长度
        0x06,//指令码
        buffer_id,
        (page_id>>8)&0xff,
        page_id&0xff,
        0x00, 0x09//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}

//返回确认码
uint8_t AS608_Search(uint8_t buffer_id,uint16_t start_page,uint16_t page_num){

    uint8_t rsp_buffer[16] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x08,//长度
        0x1B,//指令码
        buffer_id,
        (start_page>>8)&0xff,start_page&0xff,
        (page_num>>8)&0xff,page_num&0xff,
        0x00, 0x00//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}



uint8_t AS608_DeletChar(uint16_t page_id,uint16_t num){
    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_buffer[] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x07,//长度
        0x0c,//指令码
        (page_id>>8)&0xff,page_id&0xff,
        (num>>8)&0xff,num&0xff,
        0x00, 0x00//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}

uint8_t AS608_Empty(void){
    uint8_t rsp_buffer[12] = {0};
    uint8_t cmd_buffer[12] = {
        0xEF, 0x01,//包头
        0xFF, 0xFF, 0xFF, 0xFF,//默认地址
        0x01,//包标识
        0x00, 0x07,//长度
        0x0d,//指令码
        0x00, 0x11//校验和
    };
    uint8_t rsp_len = sizeof(rsp_buffer);
    uint8_t cmd_len = sizeof(cmd_buffer);

    uint8_t sum = 0;
    for (uint8_t i=6; i<cmd_len-2; i++) {
        sum += cmd_buffer[i];
    }
    cmd_buffer[cmd_len-2] = (sum >> 8)&0xff;
    cmd_buffer[cmd_len-1] = sum&0xff;

    AS608_Transceive(cmd_buffer,cmd_len, rsp_buffer, rsp_len);

    return rsp_buffer[9];
}




