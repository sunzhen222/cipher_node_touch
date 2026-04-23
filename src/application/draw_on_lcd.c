#include "draw_on_lcd.h"
#include "stdio.h"
#include "string.h"
#include "drv_lcd.h"
#include "assert.h"
#include "user_delay.h"
#include "math.h"
#include "stdlib.h"
#include "stdarg.h"
#include "cmsis_os2.h"
#include "user_memory.h"

#define TEXT_LINE_GAP               3
#define DRAW_MAX_STRING_LEN         256
#define PAGE_MARGINS                15


void DrawImageOnLcd(uint16_t x, uint16_t y, const lv_img_dsc_t *imgDsc)
{
    uint8_t *colors;

    colors = SRAM_MALLOC(imgDsc->header.w * imgDsc->header.h * 3);
    memcpy(colors, imgDsc->data, imgDsc->data_size);
    LcdDraw(x, y, x + imgDsc->header.w - 1, y + imgDsc->header.h - 1, colors);
    SRAM_FREE(colors);
}

extern const lv_img_dsc_t cipher_node_touch_logo;

void DrawBootLogoOnLcd(void)
{
    DrawImageOnLcd(64, 160, &cipher_node_touch_logo);
    SetLcdBackLight(100);
    UserDelay(500);
}
