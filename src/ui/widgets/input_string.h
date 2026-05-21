#ifndef _INPUT_STRING_H
#define _INPUT_STRING_H

#include "stdbool.h"
#include "lvgl.h"

typedef void (*InputStringHandler_t)(const char *input);

lv_obj_t *CreateInputString(lv_obj_t *parent,
                            const char *title,
                            const char *value,
                            uint32_t maxLen,
                            bool isPassword,
                            InputStringHandler_t handler);

#endif
