#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "user_msg.h"

typedef struct {
    //lv_obj_t *checkBoxBlue;
    //lv_obj_t *checkBoxRed;
    //lv_obj_t *checkBoxPink;
    //lv_obj_t *checkBoxPurple;
    //lv_obj_t *checkBoxGreen;
    //lv_obj_t *checkBoxOrange;
    //lv_obj_t *checkBoxDeepOrange;
    lv_obj_t *checkBoxColors[7];
} WidgetColorPageValues_t;


static void WidgetColorPageInit(void);
static void WidgetColorPageDeinit(void);
static void WidgetColorPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void ColorCheckboxEventHandler(lv_event_t *e);
static lv_obj_t *CreateColorCheckbox(lv_obj_t *parent, lv_color_t color, int32_t x_ofs, int32_t y_ofs, lv_event_cb_t event_cb);

Page_t g_widgetColorPage = {
    .init = WidgetColorPageInit,
    .deinit = WidgetColorPageDeinit,
    .msgHandler = WidgetColorPageMsgHandler,
    .fullScreen = true,
};

static lv_palette_t g_colorList[] = {
    LV_PALETTE_BLUE,
    LV_PALETTE_RED,
    LV_PALETTE_PINK,
    LV_PALETTE_PURPLE,
    LV_PALETTE_GREEN,
    LV_PALETTE_ORANGE,
    LV_PALETTE_DEEP_ORANGE,
};

static void WidgetColorPageInit(void)
{
    WidgetColorPageValues_t *values = SRAM_MALLOC(sizeof(WidgetColorPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    CreateGeneralNavigationBar();

    lv_obj_t *temp;

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "Widget color");
    lv_obj_align(temp, LV_ALIGN_TOP_MID, 0, 60);

    for (uint32_t i = 0; i < sizeof(g_colorList) / sizeof(lv_palette_t); i++) {
        values->checkBoxColors[i] = CreateColorCheckbox(GetPageBackground(), lv_palette_main(g_colorList[i]), 25 + i * 40, 140, ColorCheckboxEventHandler);
        if (g_colorList[i] == DeviceSettingsGetWidgetColor()) {
            lv_obj_add_state(values->checkBoxColors[i], LV_STATE_CHECKED);
        }
    }
}

static void WidgetColorPageDeinit(void)
{
    WidgetColorPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void WidgetColorPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void ColorCheckboxEventHandler(lv_event_t *e)
{
    WidgetColorPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *cb = lv_event_get_target(e);
    bool checked = lv_obj_has_state(cb, LV_STATE_CHECKED);
    if (!checked) {
        // Prevent unchecking
        lv_obj_add_state(cb, LV_STATE_CHECKED);
        return;
    }
    for (uint32_t i = 0; i < sizeof(g_colorList) / sizeof(lv_palette_t); i++) {
        if (cb == values->checkBoxColors[i]) {
            DeviceSettingsSetWidgetColor(g_colorList[i]);
            SaveDeviceSettings();
            PubValueMsg(UI_MSG_RELOAD_THEME, 0);
        } else {
            lv_obj_remove_state(values->checkBoxColors[i], LV_STATE_CHECKED);
        }
    }
}

static lv_obj_t *CreateColorCheckbox(lv_obj_t *parent, lv_color_t color, int32_t x_ofs, int32_t y_ofs, lv_event_cb_t event_cb)
{
    lv_obj_t *cb = lv_checkbox_create(parent);
    lv_checkbox_set_text(cb, "");
    lv_obj_align(cb, LV_ALIGN_TOP_LEFT, x_ofs, y_ofs);
    lv_obj_set_style_bg_color(cb, color, 0);
    lv_obj_set_style_radius(cb, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(cb, LV_OPA_COVER, 0);

    lv_obj_set_style_border_color(cb, lv_color_hex(0x000000), LV_PART_INDICATOR);
    lv_obj_set_style_border_width(cb, 4, LV_PART_INDICATOR);
    lv_obj_set_style_outline_color(cb, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_outline_width(cb, 2, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(cb, LV_OPA_TRANSP, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(cb, LV_OPA_TRANSP, LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(cb, 8, LV_PART_INDICATOR);
    lv_obj_set_style_radius(cb, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    if (event_cb != NULL) {
        lv_obj_add_event_cb(cb, event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    }
    return cb;
}
