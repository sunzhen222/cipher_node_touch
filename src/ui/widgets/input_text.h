#ifndef _INPUT_TEXT_H
#define _INPUT_TEXT_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

typedef void (*InputTextHandler_t)(const char *input);

lv_obj_t *CreateInputText(lv_obj_t *parent,
                          const char *title,
                          const char *value,
                          uint32_t maxLen,
                          bool isPassword,
                          InputTextHandler_t handler);

#endif
