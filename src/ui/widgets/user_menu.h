
#ifndef _USER_MENU_H
#define _USER_MENU_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

typedef struct {
    const char *text;
    lv_event_cb_t handler;
} UserMenuItem_t;

lv_obj_t *CreateUserMenu(const UserMenuItem_t *items, uint32_t itemCount);
void SetUserMenuItemChecked(lv_obj_t *userMenu, uint32_t itemIndex, bool checked);
void DestroyUserMenu(lv_obj_t *userMenu);


#endif
