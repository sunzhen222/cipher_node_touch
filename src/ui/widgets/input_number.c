#include "input_number.h"
#include "user_memory.h"
#include "stdio.h"
#include "stdlib.h"
#include "ui_color.h"
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
    lv_obj_set_style_bg_color(bg, UI_COLOR_BLACK, 0);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);

    InputNumberValue_t *inputNumberValue = SRAM_MALLOC(sizeof(InputNumberValue_t));
    inputNumberValue->min = min;
    inputNumberValue->max = max;
    inputNumberValue->handler = handler;
    inputNumberValue->bg = bg;
    inputNumberValue->firstType = true;

    lv_obj_t *pad = lv_obj_create(bg);
    lv_obj_set_size(pad, 200, 200);
    lv_obj_align(pad, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *backButton = lv_button_create(pad);
    lv_obj_set_size(backButton, 50, 24);
    lv_obj_align(backButton, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_t *backImg = lv_image_create(backButton);
    lv_image_set_src(backImg, LV_SYMBOL_LEFT);
    lv_obj_set_style_text_color(backImg, UI_COLOR_WHITE, 0);
    lv_obj_align(backImg, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(backButton, BackHandler, LV_EVENT_CLICKED, inputNumberValue);

    lv_obj_t *label = lv_label_create(bg);
    lv_label_set_text(label, title);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *ta = lv_textarea_create(pad);
    lv_obj_set_size(ta, 100, 20);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_max_length(ta, GetDecimalDigits(max));
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 25);
    sprintf(string, "%lu", value);
    lv_textarea_set_text(ta, string);
    inputNumberValue->ta = ta;

    lv_obj_t *btnm = lv_buttonmatrix_create(pad);
    lv_obj_set_size(btnm, 200, 150);
    lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_buttonmatrix_set_map(btnm, g_inputNumberMap);
    lv_obj_add_event_cb(btnm, InputNumberEventHandler, LV_EVENT_VALUE_CHANGED, inputNumberValue);
    lv_obj_set_style_pad_all(btnm, 5, 0);
    lv_obj_set_style_pad_gap(btnm, 5, 0);
    lv_obj_set_style_text_color(btnm, UI_COLOR_BLACK, LV_PART_ITEMS);
    lv_obj_set_style_radius(btnm, 5, LV_PART_ITEMS);
    lv_obj_set_style_bg_color(btnm, UI_COLOR_LIGHT_GRAY, LV_PART_ITEMS | LV_STATE_PRESSED);

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
}

static void BackHandler(lv_event_t *e)
{
    InputNumberValue_t *inputNumberValue = lv_event_get_user_data(e);
    lv_obj_del(inputNumberValue->bg);
    SRAM_FREE(inputNumberValue);
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
