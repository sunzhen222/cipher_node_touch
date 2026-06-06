#ifndef _SIMULATOR_DRV_LCD_H
#define _SIMULATOR_DRV_LCD_H

#include <stdbool.h>
#include <stdint.h>

#define LCD_DISPLAY_WIDTH 320
#define LCD_DISPLAY_HEIGHT 480

void SetLcdBackLight(uint32_t brightness);
bool LcdIsOpen(void);

#endif
