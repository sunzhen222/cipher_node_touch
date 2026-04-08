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

typedef struct {
    uint16_t green_h : 3;
    uint16_t red : 5;
    uint16_t blue : 5;
    uint16_t green_l : 3;
} LcdDrawColor_t;

static void GetTrueColors(uint16_t *trueColors, const uint8_t *colorData, uint32_t pixelNum, lv_color_format_t colorFormat);

void DrawImageOnLcd(uint16_t x, uint16_t y, const lv_img_dsc_t *imgDsc)
{
    uint16_t *colors;

    colors = SRAM_MALLOC(imgDsc->header.w * imgDsc->header.h * 3);
    GetTrueColors(colors, imgDsc->data, imgDsc->header.w * imgDsc->header.h, imgDsc->header.cf);
    LcdDraw(x, y, x + imgDsc->header.w - 1, y + imgDsc->header.h - 1, colors);
    SRAM_FREE(colors);
}

extern const lv_img_dsc_t cipher_node_touch_logo;

void DrawBootLogoOnLcd(void)
{
    DrawImageOnLcd(62, 96, &cipher_node_touch_logo);
    SetLcdBackLight(100);
    UserDelay(2000);
}

static void GetTrueColors(uint16_t *trueColors, const uint8_t *colorData, uint32_t pixelNum, lv_color_format_t colorFormat)
{
    if (colorFormat == LV_COLOR_FORMAT_RGB565A8) {
        memcpy(trueColors, colorData, pixelNum * 3);
    } else if (colorFormat == LV_COLOR_FORMAT_RGB565) {
        memcpy(trueColors, colorData, pixelNum * 2);
    }
}
