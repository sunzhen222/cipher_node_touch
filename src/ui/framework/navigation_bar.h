#ifndef _NAVIGATION_BAR_H
#define _NAVIGATION_BAR_H

#include "stdint.h"
#include "stdbool.h"
#include "lvgl.h"

#define NAVIGATION_BAR_HEIGHT       32
typedef struct {
    const void *leftImgSrc;
    lv_event_cb_t leftBtnCb;
    const void *rightImgSrc;
    lv_event_cb_t rightBtnCb;
    const char *middleText;
} NavigationBar_t;

void CreateNavigationBar(const NavigationBar_t *navigationBar);
void CreateGeneralNavigationBar(void);

#endif
