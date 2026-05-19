#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "user_utils.h"
#include "user_memory.h"

#define TOUCH_POINT_DIAMETER 5

typedef struct {
    lv_obj_t *drawLayer;
    lv_obj_t *buttonClear;
} TouchTestPageValues_t;

static void TouchTestPageInit(void);
static void TouchTestPageDeinit(void);
static void TouchTestPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void TouchTestPageTouchEventHandler(lv_event_t *e);
static void TouchTestPageClearButtonEventHandler(lv_event_t *e);
static void DrawTouchPoint(lv_obj_t *parent, lv_point_t point);

Page_t g_touchTestPage = {
    .init = TouchTestPageInit,
    .deinit = TouchTestPageDeinit,
    .msgHandler = TouchTestPageMsgHandler,
    .fullScreen = true,
};

static void TouchTestPageInit(void)
{
    TouchTestPageValues_t *values = SRAM_MALLOC(sizeof(TouchTestPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    lv_obj_t *background = GetPageBackground();
    lv_obj_set_style_bg_color(background, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(background, LV_OPA_COVER, 0);
    lv_obj_add_flag(background, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(background, TouchTestPageTouchEventHandler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(background, TouchTestPageTouchEventHandler, LV_EVENT_PRESSING, NULL);

    values->drawLayer = lv_obj_create(background);
    lv_obj_remove_style_all(values->drawLayer);
    lv_obj_set_size(values->drawLayer, lv_pct(100), lv_pct(100));
    lv_obj_align(values->drawLayer, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_flag(values->drawLayer, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(values->drawLayer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(values->drawLayer, TouchTestPageTouchEventHandler, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(values->drawLayer, TouchTestPageTouchEventHandler, LV_EVENT_PRESSING, NULL);

    values->buttonClear = lv_button_create(background);
    lv_obj_set_size(values->buttonClear, 64, 36);
    lv_obj_align(values->buttonClear, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(values->buttonClear, TouchTestPageClearButtonEventHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_t *clearLabel = lv_label_create(values->buttonClear);
    lv_label_set_text(clearLabel, "Clear");
    lv_obj_center(clearLabel);
}

static void TouchTestPageDeinit(void)
{
    TouchTestPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void TouchTestPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void TouchTestPageTouchEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING) {
        return;
    }

    lv_indev_t *indev = lv_indev_active();
    if (indev == NULL) {
        return;
    }

    lv_point_t point;
    lv_indev_get_point(indev, &point);

    TouchTestPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    DrawTouchPoint(values->drawLayer, point);
}

static void TouchTestPageClearButtonEventHandler(lv_event_t *e)
{
    UNUSED(e);
    TouchTestPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_clean(values->drawLayer);
}

static void DrawTouchPoint(lv_obj_t *parent, lv_point_t point)
{
    // Draw each sampled touch coordinate as a solid white dot.
    lv_obj_t *dot = lv_obj_create(parent);
    lv_obj_remove_style_all(dot);
    lv_obj_set_size(dot, TOUCH_POINT_DIAMETER, TOUCH_POINT_DIAMETER);
    lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(dot, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dot, 0, 0);
    lv_obj_set_pos(dot, point.x - TOUCH_POINT_DIAMETER / 2, point.y - TOUCH_POINT_DIAMETER / 2);
}
