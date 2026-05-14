#include "page.h"
#include "stdio.h"
#include "stdlib.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "input_number.h"
#include "confirm_win.h"
#include "images_declare.h"
#include "lora.h"

typedef struct {
    lv_obj_t *frequencyInput;
    lv_obj_t *unsavedLabel;
    bool unsaved;
} ChatSettingsPageValues_t;


static void ChatSettingsPageInit(void);
static void ChatSettingsPageDeinit(void);
static void ChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void BackButtonHandler(lv_event_t *e);
static void ConfirmBackEventHandler(lv_event_t *e);
static void SaveBtnEventHandler(lv_event_t *e);
static void FrequencyInputEventHandler(lv_event_t *e);
static void FrequencyInputNumberHandler(uint32_t input);

static void SetUnsavedState(bool unsaved);

Page_t g_chatSettingsPage = {
    .init = ChatSettingsPageInit,
    .deinit = ChatSettingsPageDeinit,
    .msgHandler = ChatSettingsPageMsgHandler,
    .fullScreen = false,
};


static void ChatSettingsPageInit(void)
{
    char string[12];

    NavigationBar_t navigationBar = {
        .leftImgSrc = &img_back,
        .leftBtnCb = BackButtonHandler,
        .rightImgSrc = NULL,
        .rightBtnCb = NULL,
        .middleText = NULL,
    };
    CreateNavigationBar(&navigationBar);
    ChatSettingsPageValues_t *values = SRAM_MALLOC(sizeof(ChatSettingsPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    lv_obj_t *label, *button;

    button = lv_button_create(GetPageBackground());
    lv_obj_set_size(button, 60, 32);
    label = lv_label_create(button);
    lv_label_set_text(label, "Save");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -8, 8);
    lv_obj_add_event_cb(button, SaveBtnEventHandler, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Frequency");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 56);
    values->frequencyInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_one_line(values->frequencyInput, true);
    lv_obj_set_style_pad_all(values->frequencyInput, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->frequencyInput), lv_color_black(), 0);
    lv_obj_set_size(values->frequencyInput, 120, 32);
    lv_obj_align(values->frequencyInput, LV_ALIGN_TOP_LEFT, 120, 50);
    snprintf(string, sizeof(string), "%lu", DeviceSettingsGetLoraFreq());
    lv_textarea_set_text(values->frequencyInput, string);
    lv_obj_add_event_cb(values->frequencyInput, FrequencyInputEventHandler, LV_EVENT_CLICKED, NULL);

    values->unsavedLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->unsavedLabel, "* Unsaved");
    lv_obj_align(values->unsavedLabel, LV_ALIGN_TOP_MID, 0, 8);

    SetUnsavedState(false);
}

static void ChatSettingsPageDeinit(void)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void ChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void BackButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (code == LV_EVENT_CLICKED) {
        if (values->unsaved) {
            ConfirmWin_t confirmWin = {0};
            confirmWin.text = "You have unsaved changes. Are you sure to leave without saving?";
            confirmWin.OkHandler = ConfirmBackEventHandler;
            confirmWin.CancelHandler = NULL;
            CreateConfirmWin(GetPageBackground(), &confirmWin);
        } else {
            EnterPreviousPage();
        }

    }
}

static void ConfirmBackEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        EnterPreviousPage();
    }
}

static void SaveBtnEventHandler(lv_event_t *e)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t frequency = strtoul(lv_textarea_get_text(values->frequencyInput), NULL, 10);

    UNUSED(e);

    if (!values->unsaved) {
        return;
    }
    printf("frequency: %lu\n", frequency);
    DeviceSettingsSetLoraFreq(frequency);
    SaveDeviceSettings();
    LoraSettings();
    SetUnsavedState(false);
}

static void FrequencyInputEventHandler(lv_event_t *e)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t frequency = strtoul(lv_textarea_get_text(values->frequencyInput), NULL, 10);

    UNUSED(e);

    CreateInputNumber(GetPageBackground(), "LoRa Frequency", frequency, 410000000, 525000000, FrequencyInputNumberHandler);
}

static void FrequencyInputNumberHandler(uint32_t input)
{
    char string[12];
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->frequencyInput, string);
    if (input != DeviceSettingsGetLoraFreq()) {
        SetUnsavedState(true);
    }
}

static void SetUnsavedState(bool unsaved)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    values->unsaved = unsaved;
    if (unsaved) {
        lv_obj_remove_flag(values->unsavedLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(values->unsavedLabel, LV_OBJ_FLAG_HIDDEN);
    }
}