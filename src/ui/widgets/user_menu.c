#include "user_menu.h"
#include "lvgl.h"
#include "status_bar.h"
#include "images_declare.h"
#include "user_utils.h"
#include "page.h"
#include "widgets_line.h"
#include "navigation_bar.h"
#include "user_memory.h"

#define HEADER_HEIGHT       30

typedef struct {
    lv_obj_t **checkSymbol;
} UserMenuValue_t;

lv_obj_t *CreateUserMenu(const UserMenuItem_t *items, uint32_t itemCount)
{
    CreateGeneralNavigationBar();
    lv_obj_t *bg = lv_obj_create(GetPageBackground());
    UserMenuValue_t *values = SRAM_MALLOC(sizeof(UserMenuValue_t));
    lv_obj_set_user_data(bg, values);
    values->checkSymbol = SRAM_MALLOC(sizeof(lv_obj_t *) * itemCount);
    lv_obj_align(bg, LV_ALIGN_TOP_LEFT, 0, HEADER_HEIGHT);
    Page_t *page = GetCurrentPage();
    int32_t height = lv_display_get_vertical_resolution(NULL) - HEADER_HEIGHT;
    if (page->fullScreen == false) {
        height -= STATUS_BAR_HEIGHT;
    }
    lv_obj_set_size(bg, lv_display_get_horizontal_resolution(NULL), height);
    lv_obj_align(bg, LV_ALIGN_TOP_LEFT, 0, HEADER_HEIGHT);

    lv_obj_t *line = CreateLine(bg, 260);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 1);
    for (uint32_t i = 0; i < itemCount; i++) {
        lv_obj_t *btn = lv_btn_create(bg);
        lv_obj_set_size(btn, 260, 38);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, i * 40 + 2);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x000000), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x202020), LV_STATE_PRESSED);
        lv_obj_set_layout(btn, LV_LAYOUT_NONE);
        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, items[i].text);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_add_event_cb(btn, items[i].handler, LV_EVENT_CLICKED, NULL);
        line = CreateLine(bg, 260);
        lv_obj_align(line, LV_ALIGN_TOP_MID, 0, i * 40 + 41);
        values->checkSymbol[i] = lv_image_create(btn);
        lv_image_set_src(values->checkSymbol[i], LV_SYMBOL_OK);
        lv_obj_set_style_text_color(values->checkSymbol[i], lv_color_white(), 0);
        lv_obj_align(values->checkSymbol[i], LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_add_flag(values->checkSymbol[i], LV_OBJ_FLAG_HIDDEN);
    }
    return bg;
}

void SetUserMenuItemChecked(lv_obj_t *userMenu, uint32_t itemIndex, bool checked)
{
    UserMenuValue_t *values = lv_obj_get_user_data(userMenu);
    if (checked) {
        lv_obj_remove_flag(values->checkSymbol[itemIndex], LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(values->checkSymbol[itemIndex], LV_OBJ_FLAG_HIDDEN);
    }
}

void DestroyUserMenu(lv_obj_t *userMenu)
{
    UserMenuValue_t *values = lv_obj_get_user_data(userMenu);
    SRAM_FREE(values->checkSymbol);
    SRAM_FREE(values);
}

