#ifndef _SIMULATOR_LV_PORT_INDEV_H
#define _SIMULATOR_LV_PORT_INDEV_H

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

void lv_port_indev_init(void);
void SetTouchPressed(bool pressed);
void SetTouchXY(int32_t x, int32_t y);

#endif
