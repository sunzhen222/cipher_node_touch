
#ifndef _DRC_LCD_H
#define _DRC_LCD_H

#include "stdint.h"
#include "stdbool.h"

#define LCD_DISPLAY_WIDTH       320
#define LCD_DISPLAY_HEIGHT      480

void LcdInit(void);
void LcdOpen(void);
void LcdClose(void);
bool LcdIsOpen(void);
void SetLcdBackLight(uint32_t brightness);
void LcdReset(void);
void LcdDraw(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, const void *map);
void LcdClear(uint32_t color);

#endif
