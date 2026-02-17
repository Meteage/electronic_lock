#ifndef __AS608_H__
#define __AS608_H__

#include <stdint.h>

//返回确认码
uint8_t AS608_GetImage(void);

//返回确认码
uint8_t AS608_GenChar(uint8_t buffer_id);

uint8_t AS608_Match();

uint8_t AS608_RegModel();

uint8_t AS608_StoreChar(uint8_t buffer_id,uint8_t page_id);

uint8_t AS608_Search(uint8_t buffer_id,uint16_t start_page,uint16_t page_num);

uint8_t AS608_DeletChar(uint16_t page_id,uint16_t num);

uint8_t AS608_Empty(void);


#endif
