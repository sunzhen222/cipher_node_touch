#include "navigation_bar.h"
#include "images_declare.h"
#include "page.h"

static void BackButtonHandler(lv_event_t *e);

void CreateNavigationBar(const NavigationBar_t *navigationBar)
{
    lv_obj_t *img, *btn, *label;
    lv_obj_t *bg = lv_obj_create(GetPageBackground());
    lv_obj_set_size(bg, lv_display_get_horizontal_resolution(NULL), NAVIGATION_BAR_HEIGHT);
    if (navigationBar->leftImgSrc != NULL) {
        btn = lv_btn_create(bg);
        lv_obj_set_size(btn, 50, 24);
        lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_add_event_cb(btn, navigationBar->leftBtnCb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x33, 0x33, 0x33), LV_STATE_PRESSED);
        img = lv_img_create(btn);
        lv_img_set_src(img, navigationBar->leftImgSrc);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    }
    if (navigationBar->rightImgSrc != NULL) {
        btn = lv_btn_create(bg);
        lv_obj_set_size(btn, 50, 24);
        lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_add_event_cb(btn, navigationBar->rightBtnCb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x33, 0x33, 0x33), LV_STATE_PRESSED);
        img = lv_img_create(btn);
        lv_img_set_src(img, navigationBar->rightImgSrc);
        lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    }
    if (navigationBar->middleText != NULL) {
        label = lv_label_create(bg);
        lv_label_set_text(label, navigationBar->middleText);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
    }
}

void CreateGeneralNavigationBar(void)
{
    NavigationBar_t navigationBar = {
        .leftImgSrc = &img_back,
        .leftBtnCb = BackButtonHandler,
        .rightImgSrc = NULL,
        .rightBtnCb = NULL,
        .middleText = NULL,
    };
    CreateNavigationBar(&navigationBar);
}

static void BackButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        EnterPreviousPage();
    }
}

