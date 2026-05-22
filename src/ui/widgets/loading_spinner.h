
#ifndef _LOADING_SPINNER_H
#define _LOADING_SPINNER_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

lv_obj_t *CreateLoadingSpinner(lv_obj_t *parent, uint32_t timeout);
void DeleteLoadingSpinner(lv_obj_t *spinner);

#endif
