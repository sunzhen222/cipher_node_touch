#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "drv_lcd.h"

typedef struct {
    uint32_t brightness;
    bool valueChanged;
} LcdBrightnessPageValues_t;


static void LcdBrightnessPageInit(void);
static void LcdBrightnessPageDeinit(void);
static void LcdBrightnessPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

static void SliderBrightnessEventCallback(lv_event_t *e);

Page_t g_lcdBrightnessPage = {
    .init = LcdBrightnessPageInit,
    .deinit = LcdBrightnessPageDeinit,
    .msgHandler = LcdBrightnessPageMsgHandler,
    .fullScreen = true,
};

static void LcdBrightnessPageInit(void)
{
    LcdBrightnessPageValues_t *values = SRAM_MALLOC(sizeof(LcdBrightnessPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    CreateGeneralNavigationBar();

    lv_obj_t *temp;

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "LCD Brightness");
    lv_obj_align(temp, LV_ALIGN_TOP_MID, 0, 60);

    temp = lv_slider_create(GetPageBackground());
    lv_slider_set_min_value(temp, 10);
    lv_slider_set_max_value(temp, 100);
    lv_slider_set_value(temp, DeviceSettingsGetBrightness(), LV_ANIM_OFF);
    lv_obj_set_width(temp, 272);
    lv_obj_set_height(temp, 20);
    lv_obj_align(temp, LV_ALIGN_TOP_MID, 0, 140);
    lv_obj_add_event_cb(temp, SliderBrightnessEventCallback, LV_EVENT_VALUE_CHANGED, NULL);
}

static void LcdBrightnessPageDeinit(void)
{
    LcdBrightnessPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values->valueChanged) {
        SaveDeviceSettings();
    }
    SRAM_FREE(values);
}

static void LcdBrightnessPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void SliderBrightnessEventCallback(lv_event_t *e)
{
    LcdBrightnessPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *slider = lv_event_get_target(e);
    values->brightness = lv_slider_get_value(slider);
    printf("LCD Brightness: %lu\n", values->brightness);

    SetLcdBackLight(values->brightness);
    DeviceSettingsSetBrightness(values->brightness);
    values->valueChanged = true;
}
