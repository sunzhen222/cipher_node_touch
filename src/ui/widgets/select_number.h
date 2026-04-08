
#ifndef _SELECT_NUMBER_H
#define _SELECT_NUMBER_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

typedef void (*SelectNumberHandler_t)(uint32_t input);

lv_obj_t *CreateSelectNumber(lv_obj_t *parent, uint32_t value, uint32_t min, uint32_t max, SelectNumberHandler_t handler);
void DestroySelectNumber(lv_obj_t *obj);
uint32_t GetSelectNumberValue(lv_obj_t *obj);
void SetSelectNumberValue(lv_obj_t *obj, uint32_t value);
void SetSelectNumberMax(lv_obj_t *obj, uint32_t max);

#endif
