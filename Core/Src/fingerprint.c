#include "fingerprint.h"
#include "as608.h"



/**
  * @brief 获取指纹并生成特征
  * @param buffer_id: 缓冲区ID (0x01 或 0x02)
  * @retval 0: 成功, 1: 失败, 2: 无手指
  */
uint8_t Fingerprint_GetChar(uint8_t buffer_id)
{
    // 检测是否有手指
    if(GET_TOUCH == 0)
    {
        return FINGERPRINT_NO_FINGER;
    }
    
    HAL_Delay(50);  // 等待手指稳定
    
    uint8_t rsp = AS608_GetImage();
    if(rsp != 0)
    {
        return FINGERPRINT_FAIL;
    }
    
    rsp = AS608_GenChar(buffer_id);
    if(rsp != 0)
    {
        return FINGERPRINT_FAIL;
    }
    
    // 等待手指松开
    while(GET_TOUCH)
    {
        HAL_Delay(10);
    }
    
    return FINGERPRINT_OK;
}

/**
  * @brief 指纹录入
  * @param page_id: 存储位置ID
  * @retval 0: 成功, 1: 失败
  */
uint8_t Fingerprint_Enroll(uint8_t page_id)
{
    static uint8_t state = 0;
    uint8_t rsp;
    
    if(state == 0)
    {
        // 录入第一个特征
        rsp = Fingerprint_GetChar(0x01);
        if(rsp == FINGERPRINT_OK)
        {
            state = 1;
        }
        else if(rsp == FINGERPRINT_FAIL)
        {
            state = 0;  // 失败时重置状态
            return FINGERPRINT_FAIL;
        }
        // FINGERPRINT_NO_FINGER 不处理，继续等待
    }
    else if(state == 1)
    {
        // 录入第二个特征
        rsp = Fingerprint_GetChar(0x02);
        if(rsp == FINGERPRINT_OK)
        {
            // 合并特征
            rsp = AS608_RegModel();
            if(rsp == 0)
            {
                // 存储指纹
                rsp = AS608_StoreChar(0x01, page_id);
                state = 0;  // 重置状态
                return (rsp == 0) ? FINGERPRINT_OK : FINGERPRINT_FAIL;
            }
        }
        else if(rsp == FINGERPRINT_FAIL)
        {
            state = 0;  // 失败时重置状态
            return FINGERPRINT_FAIL;
        }
        // FINGERPRINT_NO_FINGER 不处理，继续等待
    }
    
    return FINGERPRINT_FAIL;  // 仍在录入过程中
}

/**
  * @brief 指纹识别
  * @retval 0: 成功, 1: 失败, 2: 无手指
  */
uint8_t Fingerprint_Identify(void)
{
    uint8_t rsp;
    
    rsp = Fingerprint_GetChar(0x01);
    if(rsp != FINGERPRINT_OK)
    {
        return rsp;  // 返回失败或无手指
    }
    
    rsp = AS608_Search(0x01, 0x00, 300);
    return (rsp == 0) ? FINGERPRINT_OK : FINGERPRINT_FAIL;
}

uint8_t Fingerprint_Delete(uint8_t id){
    return AS608_DeletChar(id, 1);
};

uint8_t Fingerprint_Empty(void){
    return AS608_Empty();
};