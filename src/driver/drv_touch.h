
#ifndef _DRV_TOUCH_H
#define _DRV_TOUCH_H

#include "stdint.h"
#include "stdbool.h"

#define TOUCH_PAD_RES_X                 320
#define TOUCH_PAD_RES_Y                 240

typedef void (*TouchPadIntCallbackFunc_t)(void);

typedef struct {
    bool touch;
    bool continueReading;
    uint16_t x;
    uint16_t y;
} TouchStatus_t;


void TouchInit(TouchPadIntCallbackFunc_t func);
void TouchReset(void);
void TouchOnOff(bool on);
int32_t TouchGetStatus(TouchStatus_t *status);
void TouchPadIntHandler(void);


#endif
