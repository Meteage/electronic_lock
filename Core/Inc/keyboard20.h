#ifndef __KEYBOARD20_H__
#define __KEYBOARD20_H__

typedef enum key{
    KEY_NULL,
    KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,
    KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,
    KEY_F1,KEY_F2,KEY_J,KEY_X,
    KEY_UP,KEY_DOWN,KEY_LEFT,KEY_RIGHT,
    KEY_ESC,KEY_ENT,
} Key;

void KEYBOARD20_Init(void);
Key KEYBOARD20_Scan(void);

char KEYBOARD20_GetKeyValue(Key key);

#endif