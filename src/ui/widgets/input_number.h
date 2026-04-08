
#ifndef _INPUT_NUMBER_H
#define _INPUT_NUMBER_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

typedef void (*InputNumberHandler_t)(uint32_t input);

lv_obj_t *CreateInputNumber(lv_obj_t *parent, const char *title, uint32_t value, uint32_t min, uint32_t max, InputNumberHandler_t handler);


#endif
