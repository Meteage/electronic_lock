#include "at24c02.h"
#include "software_i2c.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>

#define AT24C02_ADDRESS 0xA0

uint8_t AT24C02_IsBusy(void){
    SOFTWARE_I2C_Start();

    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS);

    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 1;
    }

    SOFTWARE_I2C_Stop();

    return 0;
}

uint8_t AT24C02_WriteByte(uint8_t byte_address, uint8_t data){
    while (AT24C02_IsBusy()); //等待写结束

    SOFTWARE_I2C_Start();

    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS);

    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_SendByte(byte_address);
    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_SendByte(data);
    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_Stop();
    return 1;
}

uint8_t AT24C02_WritePage(uint8_t byte_address, uint8_t *data, uint8_t len){
    while (AT24C02_IsBusy()); //等待写结束

    SOFTWARE_I2C_Start();

    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS);
    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_SendByte(byte_address);
    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }

    for(uint8_t i = 0; i < len; i++){
        SOFTWARE_I2C_SendByte(data[i]);
        if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
            SOFTWARE_I2C_Stop();
            return 0;
        }
    }
    return 1;
}

uint8_t AT24C02_ReadByte(uint8_t data_address,uint8_t *byte){
    while (AT24C02_IsBusy());
    SOFTWARE_I2C_Start();

    //伪写
    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS);

    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_SendByte(data_address);
    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }

    //读
    SOFTWARE_I2C_Start();
    
    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS|0x1);
   
    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }

    *byte = SOFTWARE_I2C_ReceiveByte();

    SOFTWARE_I2C_Stop();
    return 1;
}

uint8_t AT24C02_ReadData(uint8_t data_address,uint8_t *data,uint8_t len){
    while (AT24C02_IsBusy());
    SOFTWARE_I2C_Start();

    //伪写
    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS);
    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }

    SOFTWARE_I2C_SendByte(data_address);
    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }
    //读
    SOFTWARE_I2C_Start();

    SOFTWARE_I2C_SendByte(AT24C02_ADDRESS|0x1);
    if(SOFTWARE_I2C_ReceiveAck() == 1){
        SOFTWARE_I2C_Stop();
        return 0;
    }

    for (uint8_t i=0; i<len; i++) {
        data[i] = SOFTWARE_I2C_ReceiveByte();
        if(i == len-1) {
            SOFTWARE_I2C_SendAck(1);  // 最后一个字节发 NACK
        } else {
            SOFTWARE_I2C_SendAck(0);  // 前面的发 ACK
        }
    }
    
    SOFTWARE_I2C_Stop();
    return 1;
}
