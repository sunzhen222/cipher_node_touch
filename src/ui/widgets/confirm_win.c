#include "confirm_win.h"
#include "user_assert.h"

#define CONFIRM_WIN_WIDTH      220
#define CONFIRM_WIN_HEIGHT     150

static void DefaultCancelHandler(lv_event_t *e);

lv_obj_t *CreateConfirmWin(lv_obj_t *parent, const ConfirmWin_t *desc)
{
    lv_obj_t *bg, *win, *label, *btn, *img;

    bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_50, 0);

    win = lv_obj_create(bg);
    lv_obj_set_size(win, CONFIRM_WIN_WIDTH, CONFIRM_WIN_HEIGHT);
    lv_obj_align(win, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(win, lv_color_make(0xEE, 0xEE, 0xEE), 0);
    lv_obj_set_style_radius(win, 10, 0);

    label = lv_label_create(win);
    lv_label_set_text(label, desc->text);
    lv_obj_set_width(label, CONFIRM_WIN_WIDTH - 40);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_update_layout(label);
    int32_t labelHeight = lv_obj_get_height(label);
    ASSERT(CONFIRM_WIN_HEIGHT - labelHeight - 50 > 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, (CONFIRM_WIN_HEIGHT - labelHeight - 50) / 2);

    if (desc->OkHandler == NULL && desc->CancelHandler == NULL) {
        btn = lv_btn_create(win);
        lv_obj_set_size(btn, 60, 30);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -20);
        img = lv_image_create(btn);
        lv_image_set_src(img, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(img, lv_color_white(), 0);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(btn, DefaultCancelHandler, LV_EVENT_CLICKED, bg);
    } else {
        btn = lv_btn_create(win);
        lv_obj_set_size(btn, 60, 30);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
        img = lv_image_create(btn);
        lv_image_set_src(img, LV_SYMBOL_OK);
        lv_obj_set_style_text_color(img, lv_color_white(), 0);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(btn, desc->OkHandler, LV_EVENT_CLICKED, bg);

        btn = lv_btn_create(win);
        lv_obj_set_size(btn, 60, 30);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
        img = lv_image_create(btn);
        lv_image_set_src(img, LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(img, lv_color_white(), 0);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
        if (desc->CancelHandler == NULL) {
            lv_obj_add_event_cb(btn, DefaultCancelHandler, LV_EVENT_CLICKED, bg);
        } else {
            lv_obj_add_event_cb(btn, desc->CancelHandler, LV_EVENT_CLICKED, bg);
        }
    }
    return bg;
}

static void DefaultCancelHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *bg = lv_event_get_user_data(e);
        lv_obj_del(bg);
    }
}
