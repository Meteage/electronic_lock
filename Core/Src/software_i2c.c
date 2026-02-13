#include "software_i2c.h"
#include "cmsis_gcc.h"
#include "stm32f1xx_hal.h"

static void I2C_Delay(void) {
    __NOP();
    __NOP();
}

void SOFTWARE_I2C_Init(void){
    SCL_W(1);SDA_W(1);//默认都为高电平
}

//I2c工作时，默认保持SDA:x,SCL:0
//I2c不工作，默认保持SDA:1,SCL:1

void SOFTWARE_I2C_Start(void){
    SDA_W(1);
    //debug
    I2C_Delay();
    SCL_W(1);
    I2C_Delay();
    SDA_W(0);
    I2C_Delay();
    SCL_W(0);//默认保持SCL为低
}

void SOFTWARE_I2C_Stop(void){
    SDA_W(0);
    //debug
    I2C_Delay();
    SCL_W(1);
    I2C_Delay();
    SDA_W(1);
    //debug
    I2C_Delay();
}

//发送一个字节 8bit 高位先发
void SOFTWARE_I2C_SendByte(uint8_t byte){
    for(int i=0;i<8;i++){
        //变化SDA
        SDA_W((byte<<i)&0x80);
        I2C_Delay();
        SCL_W(1);
        I2C_Delay();
        SCL_W(0);
        I2C_Delay();
    }
}

//这里时序不对
uint8_t SOFTWARE_I2C_ReceiveByte(){
    uint8_t byte = 0;
    SDA_W(1); //释放SDA线 
    for(int i=0;i<8;i++){
        SCL_W(1);
        I2C_Delay();  //读取时序应该放中间
        if(SDA_R) byte |= (0x80 >> i);
        SCL_W(0);
        I2C_Delay();
    }
    return byte;
}

void SOFTWARE_I2C_SendAck(uint8_t ack){
    SDA_W(ack);
    I2C_Delay();

	SCL_W(1);
    I2C_Delay();
	SCL_W(0);
    I2C_Delay();
}

uint8_t SOFTWARE_I2C_ReceiveAck(void){
	uint8_t ack = 1;
	SDA_W(1);//释放SDA线
    I2C_Delay();

	SCL_W(1);
    I2C_Delay();
	ack = SDA_R;
	SCL_W(0);
    I2C_Delay();

	return ack;
}



// 发送带设备地址的消息
uint8_t SOFTWARE_I2C_WriteToDevice(uint8_t devAddr, uint8_t *data, uint8_t len){
    SOFTWARE_I2C_Start();
    
    // 1. 发送设备地址（写模式）
    SOFTWARE_I2C_SendByte(devAddr);
    if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
        SOFTWARE_I2C_Stop();
        return 0;
    }
    
    // 2. 发送数据
    for(uint8_t i = 0; i < len; i++){
        SOFTWARE_I2C_SendByte(data[i]);
        if(SOFTWARE_I2C_ReceiveAck() == 1) {  // NACK
            SOFTWARE_I2C_Stop();
            return 0;
        }
    }
    
    SOFTWARE_I2C_Stop();
    return 1;
}