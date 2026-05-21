#include "input_string.h"
#include "user_memory.h"
#include "user_utils.h"

typedef struct {
    InputStringHandler_t handler;
    lv_obj_t *bg;
    lv_obj_t *ta;
} InputStringValue_t;

static void InputStringKeyboardEventHandler(lv_event_t *e);
static void BackHandler(lv_event_t *e);
static void KeepTaFocusEventHandler(lv_event_t *e);
static void FocusTextarea(InputStringValue_t *inputStringValue);

lv_obj_t *CreateInputString(lv_obj_t *parent,
                            const char *title,
                            const char *value,
                            uint32_t maxLen,
                            bool isPassword,
                            InputStringHandler_t handler)
{
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    InputStringValue_t *inputStringValue = SRAM_MALLOC(sizeof(InputStringValue_t));
    inputStringValue->handler = handler;
    inputStringValue->bg = bg;

    lv_obj_t *pad = lv_obj_create(bg);
    lv_obj_set_size(pad, 320, 320);
    lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(pad, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *backButton = lv_button_create(pad);
    lv_obj_set_size(backButton, 48, 48);
    lv_obj_align(backButton, LV_ALIGN_TOP_LEFT, 8, 0);
    lv_obj_remove_flag(backButton, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_set_style_bg_color(backButton, lv_color_black(), 0);
    lv_obj_set_style_bg_color(backButton, lv_color_make(0x33, 0x33, 0x33), LV_STATE_PRESSED);
    lv_obj_t *backImg = lv_image_create(backButton);
    lv_image_set_src(backImg, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(backImg, lv_color_white(), 0);
    lv_obj_align(backImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(backButton, BackHandler, LV_EVENT_CLICKED, inputStringValue);

    lv_obj_t *label = lv_label_create(bg);
    lv_label_set_text(label, title);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 48);

    lv_obj_t *ta = lv_textarea_create(pad);
    lv_obj_set_size(ta, 288, 40);
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_pad_all(ta, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(ta), lv_color_black(), 0);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, maxLen);
    lv_textarea_set_password_mode(ta, isPassword);
    lv_textarea_set_text(ta, value == NULL ? "" : value);
    inputStringValue->ta = ta;

    lv_obj_t *keyboard = lv_keyboard_create(pad);
    lv_obj_set_size(keyboard, 320, 250);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_keyboard_set_textarea(keyboard, ta);
    lv_obj_add_event_cb(keyboard, InputStringKeyboardEventHandler, LV_EVENT_READY, inputStringValue);
    lv_obj_add_event_cb(keyboard, InputStringKeyboardEventHandler, LV_EVENT_CANCEL, inputStringValue);

    lv_obj_add_event_cb(bg, KeepTaFocusEventHandler, LV_EVENT_CLICKED, inputStringValue);
    lv_obj_add_event_cb(pad, KeepTaFocusEventHandler, LV_EVENT_CLICKED, inputStringValue);
    FocusTextarea(inputStringValue);

    return bg;
}

static void InputStringKeyboardEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    InputStringValue_t *inputStringValue = lv_event_get_user_data(e);

    if (code == LV_EVENT_READY) {
        InputStringHandler_t handler = inputStringValue->handler;
        if (handler != NULL) {
            handler(lv_textarea_get_text(inputStringValue->ta));
        }
    }

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_del(inputStringValue->bg);
        SRAM_FREE(inputStringValue);
    }
}

static void BackHandler(lv_event_t *e)
{
    InputStringValue_t *inputStringValue = lv_event_get_user_data(e);
    lv_obj_del(inputStringValue->bg);
    SRAM_FREE(inputStringValue);
}

static void KeepTaFocusEventHandler(lv_event_t *e)
{
    InputStringValue_t *inputStringValue = lv_event_get_user_data(e);
    FocusTextarea(inputStringValue);
}

static void FocusTextarea(InputStringValue_t *inputStringValue)
{
    if (inputStringValue == NULL || inputStringValue->ta == NULL) {
        return;
    }

    lv_group_focus_obj(inputStringValue->ta);
    lv_obj_add_state(inputStringValue->ta, LV_STATE_FOCUSED);
}
