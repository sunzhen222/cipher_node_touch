#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "navigation_bar.h"
#include "status_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "images_declare.h"

#define INPUT_BAR_HEIGHT        50
#define KEYBOARD_HEIGHT         160
#define SEND_BTN_WIDTH          52
#define INPUT_BAR_GAP           4

typedef struct {
    lv_obj_t *chatList;
    lv_obj_t *inputBar;
    lv_obj_t *inputTa;
    lv_obj_t *keyboard;
    lv_obj_t *sendBtn;
} MqttChatPageValues_t;


static void MqttChatPageInit(void);
static void MqttChatPageDeinit(void);
static void MqttChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void MqttChatLayout(void);
static void InputTaEventHandler(lv_event_t *e);
static void InputKeyboardEventHandler(lv_event_t *e);
static void ChatListEventHandler(lv_event_t *e);
static void InputSendBtnEventHandler(lv_event_t *e);
static void UpdateSendButtonState(MqttChatPageValues_t *values);
static void HideInputKeyboardAndRestoreLayout(MqttChatPageValues_t *values);
static void BackButtonHandler(lv_event_t *e);
static void MqttSettingsButtonHandler(lv_event_t *e);


Page_t g_mqttChatPage = {
    .init = MqttChatPageInit,
    .deinit = MqttChatPageDeinit,
    .msgHandler = MqttChatPageMsgHandler,
    .fullScreen = false,
};

static void MqttChatPageInit(void)
{
    NavigationBar_t navigationBar = {
        .leftImgSrc = &img_back,
        .leftBtnCb = BackButtonHandler,
        .rightImgSrc = &img_settings,
        .rightBtnCb = MqttSettingsButtonHandler,
        .middleText = "MQTT Chat",
    };
    CreateNavigationBar(&navigationBar);

    MqttChatPageValues_t *values = SRAM_MALLOC(sizeof(MqttChatPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    lv_obj_set_scrollbar_mode(GetPageBackground(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLLABLE);
    values->chatList = NULL;
    values->inputBar = NULL;
    values->inputTa = NULL;
    values->keyboard = NULL;
    values->sendBtn = NULL;

    MqttChatLayout();
}

static void MqttChatPageDeinit(void)
{
    MqttChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void MqttChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(code);
    UNUSED(data);
    UNUSED(dataLen);

    // TODO: Add MQTT UI message handling after MQTT transport and topic model are finalized.
}

static void MqttChatLayout(void)
{
    MqttChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (values->chatList != NULL) {
        lv_obj_delete(values->chatList);
        values->chatList = NULL;
    }
    if (values->inputBar != NULL) {
        lv_obj_delete(values->inputBar);
        values->inputBar = NULL;
    }
    if (values->keyboard != NULL) {
        lv_obj_delete(values->keyboard);
        values->keyboard = NULL;
    }

    values->chatList = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->chatList,
                    lv_display_get_horizontal_resolution(NULL),
                    lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->chatList, LV_ALIGN_TOP_LEFT, 0, NAVIGATION_BAR_HEIGHT);
    lv_obj_set_scrollbar_mode(values->chatList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(values->chatList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(values->chatList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(values->chatList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_PRESSED, values);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_CLICKED, values);
    // TODO: Add MQTT message item rendering after topic/session model is finalized.

    values->inputBar = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->inputBar, lv_display_get_horizontal_resolution(NULL), INPUT_BAR_HEIGHT);
    lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(values->inputBar, lv_color_hex(0x222222), 0);
    lv_obj_set_style_pad_left(values->inputBar, 4, 0);
    lv_obj_set_style_pad_right(values->inputBar, 4, 0);
    lv_obj_set_style_pad_top(values->inputBar, 4, 0);
    lv_obj_set_style_pad_bottom(values->inputBar, 4, 0);
    lv_obj_set_style_border_width(values->inputBar, 0, 0);

    values->inputTa = lv_textarea_create(values->inputBar);
    lv_obj_set_size(values->inputTa,
                    lv_display_get_horizontal_resolution(NULL) - SEND_BTN_WIDTH - INPUT_BAR_GAP * 3,
                    INPUT_BAR_HEIGHT - 8);
    lv_obj_align(values->inputTa, LV_ALIGN_LEFT_MID, 0, 0);
    lv_textarea_set_one_line(values->inputTa, true);
    lv_obj_set_style_pad_all(values->inputTa, 8, 0);
    lv_textarea_set_placeholder_text(values->inputTa, "Input MQTT message");
    lv_obj_set_style_text_color(values->inputTa, lv_color_hex(0x222222), LV_PART_MAIN);
    lv_obj_set_style_text_opa(values->inputTa, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(values->inputTa, lv_color_hex(0x9A9A9A), LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_text_opa(values->inputTa, LV_OPA_COVER, LV_PART_TEXTAREA_PLACEHOLDER);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->inputTa), lv_color_hex(0x222222), 0);
    lv_obj_set_style_text_opa(lv_textarea_get_label(values->inputTa), LV_OPA_COVER, 0);
    lv_obj_add_event_cb(values->inputTa, InputTaEventHandler, LV_EVENT_ALL, values);

    values->sendBtn = lv_btn_create(values->inputBar);
    lv_obj_set_size(values->sendBtn, SEND_BTN_WIDTH, INPUT_BAR_HEIGHT - 16);
    lv_obj_align(values->sendBtn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_radius(values->sendBtn, 5, 0);
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(0x16803C), 0);
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(0x134C26), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(0x4A4A4A), LV_STATE_DISABLED);
    lv_obj_set_style_text_color(values->sendBtn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(values->sendBtn, lv_color_hex(0x9A9A9A), LV_STATE_DISABLED);
    lv_obj_add_event_cb(values->sendBtn, InputSendBtnEventHandler, LV_EVENT_CLICKED, values);

    lv_obj_t *sendLabel = lv_label_create(values->sendBtn);
    lv_label_set_text(sendLabel, "Send");
    lv_obj_center(sendLabel);

    UpdateSendButtonState(values);

    values->keyboard = lv_keyboard_create(GetPageBackground());
    lv_obj_set_size(values->keyboard, lv_display_get_horizontal_resolution(NULL), KEYBOARD_HEIGHT);
    lv_obj_align(values->keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(values->keyboard, InputKeyboardEventHandler, LV_EVENT_ALL, values);
}

static void InputTaEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatPageValues_t *values = lv_event_get_user_data(e);

    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        lv_obj_set_size(values->chatList,
                        lv_display_get_horizontal_resolution(NULL),
                        lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT - KEYBOARD_HEIGHT);
        lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, -KEYBOARD_HEIGHT);
        lv_keyboard_set_textarea(values->keyboard, values->inputTa);
        lv_obj_remove_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        UpdateSendButtonState(values);
    }
}

static void InputKeyboardEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatPageValues_t *values = lv_event_get_user_data(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_clear_state(values->inputTa, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, values->inputTa);
        HideInputKeyboardAndRestoreLayout(values);
    }
}

static void ChatListEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatPageValues_t *values = lv_event_get_user_data(e);

    if (code != LV_EVENT_PRESSED && code != LV_EVENT_CLICKED) {
        return;
    }

    if (values == NULL || values->inputTa == NULL) {
        return;
    }

    if (!lv_obj_has_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_state(values->inputTa, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, values->inputTa);
        HideInputKeyboardAndRestoreLayout(values);
    }
}

static void InputSendBtnEventHandler(lv_event_t *e)
{
    MqttChatPageValues_t *values = lv_event_get_user_data(e);
    const char *text = lv_textarea_get_text(values->inputTa);

    if (text[0] == '\0') {
        return;
    }

    // TODO: Publish text to the selected MQTT topic after MQTT session management is available.
    printf("MQTT send TODO, text: %s\n", text);
    lv_textarea_set_text(values->inputTa, "");
    UpdateSendButtonState(values);
}

static void UpdateSendButtonState(MqttChatPageValues_t *values)
{
    const char *txt = lv_textarea_get_text(values->inputTa);
    if (txt[0] == '\0') {
        lv_obj_add_state(values->sendBtn, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(values->sendBtn, LV_STATE_DISABLED);
    }
}

static void HideInputKeyboardAndRestoreLayout(MqttChatPageValues_t *values)
{
    if (values == NULL || values->keyboard == NULL || values->chatList == NULL || values->inputBar == NULL) {
        return;
    }

    lv_keyboard_set_textarea(values->keyboard, NULL);
    lv_obj_add_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(values->chatList,
                    lv_display_get_horizontal_resolution(NULL),
                    lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, 0);
}

static void BackButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        EnterPreviousPage();
    }
}

static void MqttSettingsButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // TODO: Add MQTT settings page (broker, auth, topic presets, QoS policy).
        printf("MQTT settings page not implemented yet\n");
    }
}
