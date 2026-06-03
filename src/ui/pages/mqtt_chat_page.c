#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "navigation_bar.h"
#include "status_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "images_declare.h"
#include "mqtt_chat.h"
#include "mqtt.h"
#include "ui_msg.h"
#include "background_task.h"
#include "loading_spinner.h"
#include "confirm_win.h"
#include "cJSON.h"

#define INPUT_BAR_HEIGHT                50
#define KEYBOARD_HEIGHT                 160
#define SEND_BTN_WIDTH                  52
#define INPUT_BAR_GAP                   4
#define MQTT_CHAT_BUTTON_AREA_HEIGHT    48
#define MQTT_CHAT_TOPIC                 "testtopic/chat"
#define MQTT_CHAT_PUBLISH_QOS           0
#define MQTT_CHAT_PUBLISH_RETAINED      false

typedef struct {
    lv_obj_t *connectBtn;
    lv_obj_t *connectBtnLabel;
    lv_obj_t *chatList;
    lv_obj_t *inputBar;
    lv_obj_t *inputTa;
    lv_obj_t *keyboard;
    lv_obj_t *sendBtn;
    lv_obj_t *loadingSpinner;
    bool mqttOperating;
} MqttChatPageValues_t;


static void MqttChatPageInit(void);
static void MqttChatPageDeinit(void);
static void MqttChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void MqttChatLayout(void);
static void InputTaEventHandler(lv_event_t *e);
static void InputKeyboardEventHandler(lv_event_t *e);
static void ChatListEventHandler(lv_event_t *e);
static void AddNewMqttChatLayout(MqttChatItem_t *item);
static void InputSendBtnEventHandler(lv_event_t *e);
static void UpdateSendButtonState(MqttChatPageValues_t *values);
static void UpdateConnectButtonState(MqttChatPageValues_t *values);
static void HideInputKeyboardAndRestoreLayout(MqttChatPageValues_t *values);
static void ConnectBtnEventHandler(lv_event_t *e);
static int32_t AsyncMqttConnect(const void *inData, uint32_t inDataLen);
static int32_t AsyncMqttDisconnect(const void *inData, uint32_t inDataLen);
static char *CreateMqttChatPayload(const char *username, uint32_t avatarColor, const char *text);
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
    values->connectBtn = NULL;
    values->connectBtnLabel = NULL;
    values->chatList = NULL;
    values->inputBar = NULL;
    values->inputTa = NULL;
    values->keyboard = NULL;
    values->sendBtn = NULL;
    values->loadingSpinner = NULL;
    values->mqttOperating = false;

    MqttChatLayout();
}

static void MqttChatPageDeinit(void)
{
    MqttChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values->loadingSpinner != NULL) {
        DeleteLoadingSpinner(values->loadingSpinner);
        values->loadingSpinner = NULL;
    }
    SRAM_FREE(values);
}

static void MqttChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    MqttChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    MqttChatItem_t *item;

    switch (code) {
    case UI_MSG_CODE_MQTT_CONNECT_RESULT:
    case UI_MSG_CODE_MQTT_DISCONNECT_RESULT:
        if (values == NULL || data == NULL || dataLen != sizeof(bool)) {
            break;
        }
        if (values->loadingSpinner != NULL) {
            DeleteLoadingSpinner(values->loadingSpinner);
            values->loadingSpinner = NULL;
        }
        values->mqttOperating = false;
        UpdateConnectButtonState(values);
        bool operateOk = *((bool *)data);
        if (code == UI_MSG_CODE_MQTT_CONNECT_RESULT && !operateOk) {
            ConfirmWin_t confirmWin = {0};
            confirmWin.text = "MQTT connect failed";
            CreateConfirmWin(GetPageBackground(), &confirmWin);
        }
        printf("mqtt %s %s\n",
               code == UI_MSG_CODE_MQTT_CONNECT_RESULT ? "connect" : "disconnect",
               operateOk ? "ok" : "failed");
        break;
    case UI_MSG_CODE_MQTT_CHAT_ITEM:
        if (dataLen != sizeof(MqttChatItem_t *)) {
            printf("MqttChatPageMsgHandler: invalid dataLen\n");
            break;
        }
        item = *(MqttChatItem_t **)data;
        AddNewMqttChatLayout(item);
        break;
    default:
        break;
    }
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

    values->connectBtn = lv_btn_create(GetPageBackground());
    lv_obj_set_size(values->connectBtn, 96, MQTT_CHAT_BUTTON_AREA_HEIGHT - 8);
    lv_obj_align(values->connectBtn, LV_ALIGN_TOP_RIGHT, -8, NAVIGATION_BAR_HEIGHT + 4);
    lv_obj_set_style_radius(values->connectBtn, 5, 0);
    lv_obj_set_style_bg_color(values->connectBtn, lv_color_hex(0x16803C), 0);
    lv_obj_set_style_bg_color(values->connectBtn, lv_color_hex(0x134C26), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(values->connectBtn, lv_color_hex(0x4A4A4A), LV_STATE_DISABLED);
    lv_obj_set_style_text_color(values->connectBtn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_add_event_cb(values->connectBtn, ConnectBtnEventHandler, LV_EVENT_CLICKED, values);

    values->connectBtnLabel = lv_label_create(values->connectBtn);
    lv_obj_center(values->connectBtnLabel);
    UpdateConnectButtonState(values);

    values->chatList = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->chatList,
                    lv_display_get_horizontal_resolution(NULL),
                    lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - MQTT_CHAT_BUTTON_AREA_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->chatList, LV_ALIGN_TOP_LEFT, 0, NAVIGATION_BAR_HEIGHT + MQTT_CHAT_BUTTON_AREA_HEIGHT);
    lv_obj_set_scrollbar_mode(values->chatList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(values->chatList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(values->chatList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(values->chatList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_PRESSED, values);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_CLICKED, values);

    StartGetMqttChatItem();
    MqttChatItem_t *item;
    while ((item = GetNextMqttChatItem()) != NULL) {
        AddNewMqttChatLayout(item);
    }

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
                        lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - MQTT_CHAT_BUTTON_AREA_HEIGHT - INPUT_BAR_HEIGHT - KEYBOARD_HEIGHT);
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

static void AddNewMqttChatLayout(MqttChatItem_t *item)
{
    if (item == NULL) {
        return;
    }

    MqttChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values == NULL || values->chatList == NULL) {
        return;
    }

    lv_coord_t maxBubbleWidth = lv_display_get_horizontal_resolution(NULL) - 57 * 2;
    lv_coord_t maxTextWidth = maxBubbleWidth - 20;

    lv_obj_t *row = lv_obj_create(values->chatList);
    lv_obj_set_width(row, lv_pct(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_add_event_cb(row, ChatListEventHandler, LV_EVENT_PRESSED, values);
    lv_obj_add_event_cb(row, ChatListEventHandler, LV_EVENT_CLICKED, values);

    lv_obj_t *avatar = lv_obj_create(row);
    lv_obj_set_size(avatar, 40, 40);
    lv_obj_set_style_radius(avatar, 5, 0);
    lv_obj_set_style_bg_color(avatar, lv_color_hex(item->headColor), 0);
    lv_obj_set_style_border_width(avatar, 0, 0);
    if (item->self) {
        lv_obj_align(avatar, LV_ALIGN_TOP_RIGHT, -8, 8);
    } else {
        lv_obj_align(avatar, LV_ALIGN_TOP_LEFT, 8, 8);
    }

    char avatarText[2];
    avatarText[0] = (item->name[0] != '\0') ? item->name[0] : '?';
    avatarText[1] = '\0';
    lv_obj_t *avatarLabel = lv_label_create(avatar);
    lv_label_set_text(avatarLabel, avatarText);
    lv_obj_set_style_text_color(avatarLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(avatarLabel);

    lv_obj_t *nameLabel = lv_label_create(row);
    lv_label_set_text(nameLabel, item->name);
    lv_obj_set_style_text_color(nameLabel, lv_color_hex(0x888888), 0);
    if (item->self) {
        lv_obj_align(nameLabel, LV_ALIGN_TOP_RIGHT, -64, 8);
    } else {
        lv_obj_align(nameLabel, LV_ALIGN_TOP_LEFT, 64, 8);
    }

    lv_obj_t *bubble = lv_obj_create(row);
    lv_obj_set_width(bubble, LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(bubble, maxBubbleWidth, 0);
    lv_obj_set_height(bubble, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(bubble, 10, 0);
    lv_obj_set_style_border_width(bubble, 0, 0);
    lv_obj_set_style_pad_left(bubble, 10, 0);
    lv_obj_set_style_pad_right(bubble, 10, 0);
    lv_obj_set_style_pad_top(bubble, 8, 0);
    lv_obj_set_style_pad_bottom(bubble, 8, 0);
    lv_obj_set_style_pad_row(bubble, 4, 0);
    lv_obj_set_style_bg_color(bubble, item->self ? lv_color_hex(0x95EC69) : lv_color_hex(0xFFFFFF), 0);
    if (item->self) {
        lv_obj_align(bubble, LV_ALIGN_TOP_RIGHT, -57, 24);
    } else {
        lv_obj_align(bubble, LV_ALIGN_TOP_LEFT, 57, 24);
    }

    lv_obj_t *bubbleArrow = lv_obj_create(row);
    lv_obj_set_size(bubbleArrow, 6, 6);
    lv_obj_set_style_radius(bubbleArrow, 0, 0);
    lv_obj_set_style_border_width(bubbleArrow, 0, 0);
    lv_obj_set_style_bg_color(bubbleArrow, item->self ? lv_color_hex(0x95EC69) : lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_transform_rotation(bubbleArrow, 450, 0);
    if (item->self) {
        lv_obj_align(bubbleArrow, LV_ALIGN_TOP_RIGHT, -51, 36);
    } else {
        lv_obj_align(bubbleArrow, LV_ALIGN_TOP_LEFT, 56, 36);
    }

    lv_obj_t *textLabel = lv_label_create(bubble);
    lv_obj_set_width(textLabel, LV_SIZE_CONTENT);
    lv_obj_set_style_max_width(textLabel, maxTextWidth, 0);
    lv_label_set_long_mode(textLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(textLabel, item->text);
    lv_obj_set_style_text_color(textLabel, lv_color_hex(0x222222), 0);

    lv_obj_update_layout(values->chatList);
    lv_obj_scroll_to_view(row, LV_ANIM_ON);
}

static void InputSendBtnEventHandler(lv_event_t *e)
{
    MqttChatPageValues_t *values = lv_event_get_user_data(e);
    const char *text = lv_textarea_get_text(values->inputTa);
    const char *username = "Me";
    uint32_t avatarColor = 0x2FB35A;
    char *payload;

    if (text[0] == '\0') {
        return;
    }

    MqttChatItem_t *newItem = AddMqttChatItem(username, text, true, avatarColor);
    SendUiMsg(UI_MSG_CODE_MQTT_CHAT_ITEM, &newItem, sizeof(newItem));

    payload = CreateMqttChatPayload(username, avatarColor, text);
    if (payload != NULL) {
        if (!PublishMqtt(MQTT_CHAT_TOPIC, MQTT_CHAT_PUBLISH_QOS, MQTT_CHAT_PUBLISH_RETAINED, payload)) {
            printf("MQTT publish failed, payload=%s\n", payload);
        }
        cJSON_free(payload);
    }
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

static void UpdateConnectButtonState(MqttChatPageValues_t *values)
{
    if (values == NULL || values->connectBtn == NULL || values->connectBtnLabel == NULL) {
        return;
    }

    if (values->mqttOperating) {
        lv_obj_add_state(values->connectBtn, LV_STATE_DISABLED);
    } else {
        lv_obj_remove_state(values->connectBtn, LV_STATE_DISABLED);
    }

    lv_label_set_text(values->connectBtnLabel, IsMqttConnected() ? "disconnect" : "connect");
    lv_obj_center(values->connectBtnLabel);
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
                    lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - MQTT_CHAT_BUTTON_AREA_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, 0);
}

static void ConnectBtnEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatPageValues_t *values = lv_event_get_user_data(e);

    if (code != LV_EVENT_CLICKED || values == NULL || values->mqttOperating) {
        return;
    }

    values->mqttOperating = true;
    UpdateConnectButtonState(values);
    values->loadingSpinner = CreateLoadingSpinner(GetPageBackground(), 1000000);

    if (IsMqttConnected()) {
        AsyncExecute(AsyncMqttDisconnect, NULL, 0, 0);
    } else {
        AsyncExecute(AsyncMqttConnect, NULL, 0, 0);
    }
}

static int32_t AsyncMqttConnect(const void *inData, uint32_t inDataLen)
{
    UNUSED(inData);
    UNUSED(inDataLen);

    bool connectOk = (ConnectMqtt() == MQTT_CONNECT_OK);
    SendUiMsg(UI_MSG_CODE_MQTT_CONNECT_RESULT, &connectOk, sizeof(connectOk));
    return 0;
}

static int32_t AsyncMqttDisconnect(const void *inData, uint32_t inDataLen)
{
    UNUSED(inData);
    UNUSED(inDataLen);

    bool disconnectOk = DisconnectMqtt();
    SendUiMsg(UI_MSG_CODE_MQTT_DISCONNECT_RESULT, &disconnectOk, sizeof(disconnectOk));
    return 0;
}

static char *CreateMqttChatPayload(const char *username, uint32_t avatarColor, const char *text)
{
    cJSON *rootJson;
    char avatarColorString[7];
    char *payload;

    rootJson = cJSON_CreateObject();
    if (rootJson == NULL) {
        return NULL;
    }

    snprintf(avatarColorString, sizeof(avatarColorString), "%06lX", (unsigned long)(avatarColor & 0xFFFFFF));
    cJSON_AddStringToObject(rootJson, "name", username);
    cJSON_AddStringToObject(rootJson, "avatarColor", avatarColorString);
    cJSON_AddStringToObject(rootJson, "msg", text);
    payload = cJSON_PrintUnformatted(rootJson);
    cJSON_Delete(rootJson);

    return payload;
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
