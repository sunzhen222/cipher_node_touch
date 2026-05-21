#include "input_number.h"
#include "user_memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "user_utils.h"

typedef struct {
    uint32_t min;
    uint32_t max;
    InputNumberHandler_t handler;
    lv_obj_t *bg;
    lv_obj_t *ta;
    bool firstType;
} InputNumberValue_t;

static void InputNumberEventHandler(lv_event_t *e);
static void BackHandler(lv_event_t *e);
static void KeepTaFocusEventHandler(lv_event_t *e);
static void FocusTextarea(InputNumberValue_t *inputNumberValue);
uint8_t GetDecimalDigits(uint32_t number);

static const char *g_inputNumberMap[] = {"1", "2", "3", "\n",
                                         "4", "5", "6", "\n",
                                         "7", "8", "9", "\n",
                                         LV_SYMBOL_BACKSPACE, "0", LV_SYMBOL_NEW_LINE, ""
                                        };

lv_obj_t *CreateInputNumber(lv_obj_t *parent, const char *title, uint32_t value, uint32_t min, uint32_t max, InputNumberHandler_t handler)
{
    char string[12];

    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, lv_obj_get_width(parent), lv_obj_get_height(parent));
    lv_obj_set_style_bg_color(bg, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
    lv_obj_add_flag(bg, LV_OBJ_FLAG_CLICKABLE);

    InputNumberValue_t *inputNumberValue = SRAM_MALLOC(sizeof(InputNumberValue_t));
    inputNumberValue->min = min;
    inputNumberValue->max = max;
    inputNumberValue->handler = handler;
    inputNumberValue->bg = bg;
    inputNumberValue->firstType = false;

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
    lv_obj_add_event_cb(backButton, BackHandler, LV_EVENT_CLICKED, inputNumberValue);

    lv_obj_t *label = lv_label_create(bg);
    lv_label_set_text(label, title);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 48);

    lv_obj_t *ta = lv_textarea_create(pad);
    lv_obj_set_style_text_color(lv_textarea_get_label(ta), lv_color_black(), 0);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, GetDecimalDigits(max));
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_size(ta, 130, 32);
    lv_obj_set_style_pad_all(ta, 8, 0);
    sprintf(string, "%lu", value);
    lv_textarea_set_text(ta, string);
    inputNumberValue->ta = ta;

    lv_obj_t *btnm = lv_buttonmatrix_create(pad);
    lv_obj_set_size(btnm, 320, 240);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_remove_flag(btnm, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_buttonmatrix_set_map(btnm, g_inputNumberMap);
    lv_obj_add_event_cb(btnm, InputNumberEventHandler, LV_EVENT_VALUE_CHANGED, inputNumberValue);
    lv_obj_set_style_pad_all(btnm, 5, 0);
    lv_obj_set_style_pad_gap(btnm, 5, 0);
    lv_obj_set_style_text_color(btnm, lv_color_black(), LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 10, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(btnm, lv_color_make(0xEE, 0xEE, 0xEE), LV_PART_ITEMS | LV_STATE_PRESSED);

    lv_obj_add_event_cb(bg, KeepTaFocusEventHandler, LV_EVENT_CLICKED, inputNumberValue);
    lv_obj_add_event_cb(pad, KeepTaFocusEventHandler, LV_EVENT_CLICKED, inputNumberValue);
    FocusTextarea(inputNumberValue);

    return bg;
}

static void InputNumberEventHandler(lv_event_t * e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    InputNumberValue_t *inputNumberValue = lv_event_get_user_data(e);
    const char *txt = lv_buttonmatrix_get_button_text(obj, lv_buttonmatrix_get_selected_button(obj));

    if (lv_strcmp(txt, LV_SYMBOL_BACKSPACE) == 0) {
        lv_textarea_delete_char(inputNumberValue->ta);
    } else if (lv_strcmp(txt, LV_SYMBOL_NEW_LINE) == 0) {
        uint32_t value = atoi(lv_textarea_get_text(inputNumberValue->ta));
        if (value < inputNumberValue->min || value > inputNumberValue->max) {
            return;
        }
        InputNumberHandler_t handler = inputNumberValue->handler;
        if (handler != NULL) {
            handler(value);
        }
        lv_obj_del(inputNumberValue->bg);
        SRAM_FREE(inputNumberValue);
    } else {
        if (inputNumberValue->firstType) {
            lv_textarea_set_text(inputNumberValue->ta, txt);
            inputNumberValue->firstType = false;
        } else {
            lv_textarea_add_text(inputNumberValue->ta, txt);
        }
        uint32_t value = atoi(lv_textarea_get_text(inputNumberValue->ta));
        if (value > inputNumberValue->max) {
            char string[12];
            sprintf(string, "%lu", inputNumberValue->max);
            lv_textarea_set_text(inputNumberValue->ta, string);
        }
    }

    FocusTextarea(inputNumberValue);
}

static void BackHandler(lv_event_t *e)
{
    InputNumberValue_t *inputNumberValue = lv_event_get_user_data(e);
    lv_obj_del(inputNumberValue->bg);
    SRAM_FREE(inputNumberValue);
}

static void KeepTaFocusEventHandler(lv_event_t *e)
{
    InputNumberValue_t *inputNumberValue = lv_event_get_user_data(e);
    FocusTextarea(inputNumberValue);
}

static void FocusTextarea(InputNumberValue_t *inputNumberValue)
{
    if (inputNumberValue == NULL || inputNumberValue->ta == NULL) {
        return;
    }

    lv_group_focus_obj(inputNumberValue->ta);
    lv_obj_add_state(inputNumberValue->ta, LV_STATE_FOCUSED);
}

uint8_t GetDecimalDigits(uint32_t number)
{
    uint8_t digits = 0;

    if (number == 0) {
        return 1;
    }

    while (number != 0) {
        number /= 10;
        digits++;
    }

    return digits;
}
