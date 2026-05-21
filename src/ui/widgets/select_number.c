#include "select_number.h"
#include "stdio.h"
#include "stdlib.h"
#include "user_memory.h"

typedef struct {
    uint32_t min;
    uint32_t max;
    SelectNumberHandler_t handler;
    lv_obj_t *bg;
    lv_obj_t *ta;
} SelectNumberValue_t;

static void LeftHandler(lv_event_t *e);
static void RightHandler(lv_event_t *e);

lv_obj_t *CreateSelectNumber(lv_obj_t *parent, uint32_t value, uint32_t min, uint32_t max, SelectNumberHandler_t handler)
{
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, 85, 30);
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_TRANSP, 0);

    lv_obj_t *ta = lv_textarea_create(bg);
    lv_obj_set_size(ta, 25, 20);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, 2);
    lv_obj_align(ta, LV_ALIGN_CENTER, 0, 0);
    char buf[12];
    sprintf(buf, "%lu", value);
    lv_textarea_set_text(ta, buf);

    SelectNumberValue_t *selectNumberValue;
    selectNumberValue = SRAM_MALLOC(sizeof(SelectNumberValue_t));
    selectNumberValue->min = min;
    selectNumberValue->max = max;
    selectNumberValue->handler = handler;
    selectNumberValue->bg = bg;
    selectNumberValue->ta = ta;
    lv_obj_set_user_data(bg, selectNumberValue);

    lv_obj_t *button = lv_btn_create(bg);
    lv_obj_set_size(button, 30, 30);
    lv_obj_align(button, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_t *backImg = lv_image_create(button);
    lv_image_set_src(backImg, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(backImg, lv_color_white(), 0);
    lv_obj_align(backImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(button, LeftHandler, LV_EVENT_CLICKED, selectNumberValue);

    button = lv_btn_create(bg);
    lv_obj_set_size(button, 30, 30);
    lv_obj_align(button, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_TRANSP, 0);
    lv_obj_t *rightImg = lv_image_create(button);
    lv_image_set_src(rightImg, LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_color(rightImg, lv_color_white(), 0);
    lv_obj_align(rightImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(button, RightHandler, LV_EVENT_CLICKED, selectNumberValue);

    return bg;
}

void DestroySelectNumber(lv_obj_t *obj)
{
    SelectNumberValue_t *selectNumberValue = lv_obj_get_user_data(obj);
    SRAM_FREE(selectNumberValue);
}

uint32_t GetSelectNumberValue(lv_obj_t *obj)
{
    SelectNumberValue_t *selectNumberValue = lv_obj_get_user_data(obj);
    lv_obj_t *ta = selectNumberValue->ta;
    return atoi(lv_textarea_get_text(ta));
}

void SetSelectNumberValue(lv_obj_t *obj, uint32_t value)
{
    SelectNumberValue_t *selectNumberValue = lv_obj_get_user_data(obj);
    lv_obj_t *ta = selectNumberValue->ta;
    char buf[12];
    sprintf(buf, "%lu", value);
    lv_textarea_set_text(ta, buf);
}

void SetSelectNumberMax(lv_obj_t *obj, uint32_t max)
{
    SelectNumberValue_t *selectNumberValue = lv_obj_get_user_data(obj);
    selectNumberValue->max = max;
}

static void LeftHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    SelectNumberValue_t *selectNumberValue = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        char buf[12];
        lv_obj_t *ta = selectNumberValue->ta;
        uint32_t value = atoi(lv_textarea_get_text(ta));
        if (value > selectNumberValue->min) {
            value--;
        } else {
            value = selectNumberValue->max;
        }
        sprintf(buf, "%lu", value);
        lv_textarea_set_text(ta, buf);
        selectNumberValue->handler(value);
    }
}

static void RightHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    SelectNumberValue_t *selectNumberValue = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        char buf[12];
        lv_obj_t *ta = selectNumberValue->ta;
        uint32_t value = atoi(lv_textarea_get_text(ta));
        if (value < selectNumberValue->max) {
            value++;
        } else {
            value = selectNumberValue->min;
        }
        sprintf(buf, "%lu", value);
        lv_textarea_set_text(ta, buf);
        selectNumberValue->handler(value);
    }
}



