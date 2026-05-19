#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "user_utils.h"
#include "user_memory.h"
#include <stdint.h>

#define TOUCH_POINT_DIAMETER 5
#define TOUCH_POINT_MAX_CNT  1024

typedef struct {
    uint16_t x;
    uint16_t y;
} TouchPoint_t;

typedef struct {
    lv_obj_t *drawLayer;
    lv_obj_t *buttonClear;
    uint32_t pointCount;
} TouchTestPageValues_t;

static TouchPoint_t g_touchPoints[TOUCH_POINT_MAX_CNT];

static void TouchTestPageInit(void);
static void TouchTestPageDeinit(void);
static void TouchTestPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void TouchTestPageTouchEventHandler(lv_event_t *e);
static void TouchTestPageClearButtonEventHandler(lv_event_t *e);
static void TouchTestPageDrawLayerEventHandler(lv_event_t *e);
static void AddTouchPoint(TouchTestPageValues_t *values, lv_point_t point);

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
    lv_obj_add_event_cb(values->drawLayer, TouchTestPageDrawLayerEventHandler, LV_EVENT_DRAW_MAIN, NULL);

    values->pointCount = 0;

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
    AddTouchPoint(values, point);
}

static void TouchTestPageClearButtonEventHandler(lv_event_t *e)
{
    UNUSED(e);
    TouchTestPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    values->pointCount = 0;
    lv_obj_invalidate(values->drawLayer);
}

static void TouchTestPageDrawLayerEventHandler(lv_event_t *e)
{
    lv_obj_t *layerObj = lv_event_get_target(e);
    lv_layer_t *layer = lv_event_get_layer(e);
    TouchTestPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_area_t layerCoords;

    lv_obj_get_coords(layerObj, &layerCoords);

    lv_draw_rect_dsc_t dotDsc;
    lv_draw_rect_dsc_init(&dotDsc);
    dotDsc.bg_opa = LV_OPA_COVER;
    dotDsc.bg_color = lv_color_white();
    dotDsc.radius = LV_RADIUS_CIRCLE;
    dotDsc.border_width = 0;

    for (uint32_t i = 0; i < values->pointCount; i++) {
        lv_area_t dotArea;
        dotArea.x1 = g_touchPoints[i].x - TOUCH_POINT_DIAMETER / 2;
        dotArea.y1 = g_touchPoints[i].y - TOUCH_POINT_DIAMETER / 2;
        dotArea.x2 = dotArea.x1 + TOUCH_POINT_DIAMETER - 1;
        dotArea.y2 = dotArea.y1 + TOUCH_POINT_DIAMETER - 1;
        lv_area_move(&dotArea, layerCoords.x1, layerCoords.y1);
        lv_draw_rect(layer, &dotDsc, &dotArea);
    }
}

static void AddTouchPoint(TouchTestPageValues_t *values, lv_point_t point)
{
    if (values->pointCount >= TOUCH_POINT_MAX_CNT) {
        return;
    }

    g_touchPoints[values->pointCount].x = (uint16_t)point.x;
    g_touchPoints[values->pointCount].y = (uint16_t)point.y;
    values->pointCount++;

    lv_area_t area;
    area.x1 = point.x - TOUCH_POINT_DIAMETER / 2;
    area.y1 = point.y - TOUCH_POINT_DIAMETER / 2;
    area.x2 = area.x1 + TOUCH_POINT_DIAMETER - 1;
    area.y2 = area.y1 + TOUCH_POINT_DIAMETER - 1;
    lv_obj_invalidate_area(values->drawLayer, &area);
}
