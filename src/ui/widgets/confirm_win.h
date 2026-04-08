
#ifndef _CONFIRM_WIN_H
#define _CONFIRM_WIN_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

typedef struct {
    const char *text;
    lv_event_cb_t OkHandler;
    lv_event_cb_t CancelHandler;
} ConfirmWin_t;

lv_obj_t *CreateConfirmWin(lv_obj_t *parent, const ConfirmWin_t *desc);


#endif
