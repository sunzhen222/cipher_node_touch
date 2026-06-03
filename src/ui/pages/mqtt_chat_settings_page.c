#include "page.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "navigation_bar.h"
#include "status_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "input_number.h"
#include "confirm_win.h"
#include "images_declare.h"

typedef struct {
    lv_obj_t *contentObj;
    lv_obj_t *avatarPreview;
    lv_obj_t *avatarPreviewLabel;
    lv_obj_t *avatarColorDropdown;
    lv_obj_t *usernameInput;
    lv_obj_t *brokerHostInput;
    lv_obj_t *brokerPortInput;
    lv_obj_t *tlsModeInput;
    lv_obj_t *clientIdPrefixInput;
    lv_obj_t *authPrefixInput;
    lv_obj_t *subscribeTopicInput;
    lv_obj_t *subscribeQosInput;
    lv_obj_t *publishTimeoutInput;
    lv_obj_t *keyboard;
    lv_obj_t *unsavedLabel;
    bool unsaved;
} MqttChatSettingsPageValues_t;

static void MqttChatSettingsPageInit(void);
static void MqttChatSettingsPageDeinit(void);
static void MqttChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void BackButtonHandler(lv_event_t *e);
static void ConfirmBackEventHandler(lv_event_t *e);
static void SaveBtnEventHandler(lv_event_t *e);
static void AvatarColorDropdownEventHandler(lv_event_t *e);
static void TextInputEventHandler(lv_event_t *e);
static void KeyboardEventHandler(lv_event_t *e);
static void BrokerPortInputEventHandler(lv_event_t *e);
static void BrokerPortInputNumberHandler(uint32_t input);
static void TlsModeInputEventHandler(lv_event_t *e);
static void TlsModeInputNumberHandler(uint32_t input);
static void SubscribeQosInputEventHandler(lv_event_t *e);
static void SubscribeQosInputNumberHandler(uint32_t input);
static void PublishTimeoutInputEventHandler(lv_event_t *e);
static void PublishTimeoutInputNumberHandler(uint32_t input);

static lv_obj_t *CreateRowLabel(const char *text, lv_coord_t y);
static lv_obj_t *CreateTextInput(lv_coord_t y, uint32_t maxLength, const char *text);
static lv_obj_t *CreateNumberInput(lv_coord_t y, uint32_t value);
static void SetUnsavedState(bool unsaved);
static uint32_t GetSelectedAvatarColorValue(const lv_obj_t *dropdown);
static uint32_t GetAvatarColorSelectedIndex(uint32_t colorValue);
static void UpdateAvatarPreview(void);
static void UpdateUnsavedState(void);

static const char *g_avatarColorDropdownOptions = "Green\nBlue\nRed\nOrange\nPurple";
static const uint32_t g_avatarColorValues[] = {
    0x2FB35A,
    0x6155F5,
    0xFF2C2C,
    0xFF8A00,
    0x8A3FFC,
};

Page_t g_mqttChatSettingsPage = {
    .init = MqttChatSettingsPageInit,
    .deinit = MqttChatSettingsPageDeinit,
    .msgHandler = MqttChatSettingsPageMsgHandler,
    .fullScreen = false,
};

static void MqttChatSettingsPageInit(void)
{
    const lv_coord_t rowGapY = 44;
    const lv_coord_t inputYOffset = -6;
    lv_coord_t y = 24;
    lv_obj_t *label;
    lv_obj_t *button;

    NavigationBar_t navigationBar = {
        .leftImgSrc = &img_back,
        .leftBtnCb = BackButtonHandler,
        .rightImgSrc = NULL,
        .rightBtnCb = NULL,
        .middleText = NULL,
    };
    CreateNavigationBar(&navigationBar);

    MqttChatSettingsPageValues_t *values = SRAM_MALLOC(sizeof(MqttChatSettingsPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    lv_obj_set_scrollbar_mode(GetPageBackground(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLLABLE);

    button = lv_button_create(GetPageBackground());
    lv_obj_set_size(button, 60, 32);
    label = lv_label_create(button);
    lv_label_set_text(label, "Save");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -8, 8);
    lv_obj_add_event_cb(button, SaveBtnEventHandler, LV_EVENT_CLICKED, NULL);

    values->contentObj = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->contentObj,
                    lv_display_get_horizontal_resolution(NULL),
                    lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - 8);
    lv_obj_align(values->contentObj, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(values->contentObj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(values->contentObj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(values->contentObj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(values->contentObj, 0, 0);
    lv_obj_set_style_pad_all(values->contentObj, 0, 0);

    CreateRowLabel("Avatar Preview", y);
    values->avatarPreview = lv_obj_create(values->contentObj);
    lv_obj_set_size(values->avatarPreview, 40, 40);
    lv_obj_set_style_radius(values->avatarPreview, 5, 0);
    lv_obj_set_style_border_width(values->avatarPreview, 0, 0);
    lv_obj_align(values->avatarPreview, LV_ALIGN_TOP_RIGHT, -72, y + inputYOffset);
    values->avatarPreviewLabel = lv_label_create(values->avatarPreview);
    lv_obj_set_style_text_color(values->avatarPreviewLabel, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(values->avatarPreviewLabel);

    y += rowGapY + 10;
    CreateRowLabel("Avatar Color", y);
    values->avatarColorDropdown = lv_dropdown_create(values->contentObj);
    lv_obj_set_size(values->avatarColorDropdown, 120, 32);
    lv_obj_align(values->avatarColorDropdown, LV_ALIGN_TOP_RIGHT, -32, y + inputYOffset);
    lv_dropdown_set_options(values->avatarColorDropdown, g_avatarColorDropdownOptions);
    lv_dropdown_set_selected(values->avatarColorDropdown, GetAvatarColorSelectedIndex(DeviceSettingsGetMqttChatAvatarColor()));
    lv_obj_add_event_cb(values->avatarColorDropdown, AvatarColorDropdownEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    y += rowGapY;
    CreateRowLabel("Username", y);
    values->usernameInput = CreateTextInput(y, 31, DeviceSettingsGetMqttChatUsername());

    y += rowGapY;
    CreateRowLabel("Broker Host", y);
    values->brokerHostInput = CreateTextInput(y, 63, DeviceSettingsGetMqttBrokerHost());

    y += rowGapY;
    CreateRowLabel("Broker Port", y);
    values->brokerPortInput = CreateNumberInput(y, DeviceSettingsGetMqttBrokerPort());
    lv_obj_add_event_cb(values->brokerPortInput, BrokerPortInputEventHandler, LV_EVENT_CLICKED, NULL);

    y += rowGapY;
    CreateRowLabel("TLS Mode", y);
    values->tlsModeInput = CreateNumberInput(y, DeviceSettingsGetMqttTlsMode());
    lv_obj_add_event_cb(values->tlsModeInput, TlsModeInputEventHandler, LV_EVENT_CLICKED, NULL);

    y += rowGapY;
    CreateRowLabel("Client Prefix", y);
    values->clientIdPrefixInput = CreateTextInput(y, 31, DeviceSettingsGetMqttClientIdPrefix());

    y += rowGapY;
    CreateRowLabel("Auth Prefix", y);
    values->authPrefixInput = CreateTextInput(y, 31, DeviceSettingsGetMqttAuthPrefix());

    y += rowGapY;
    CreateRowLabel("Topic", y);
    values->subscribeTopicInput = CreateTextInput(y, 63, DeviceSettingsGetMqttSubscribeTopic());

    y += rowGapY;
    CreateRowLabel("QoS", y);
    values->subscribeQosInput = CreateNumberInput(y, DeviceSettingsGetMqttSubscribeQos());
    lv_obj_add_event_cb(values->subscribeQosInput, SubscribeQosInputEventHandler, LV_EVENT_CLICKED, NULL);

    y += rowGapY;
    CreateRowLabel("Pub Timeout", y);
    values->publishTimeoutInput = CreateNumberInput(y, DeviceSettingsGetMqttPublishTimeoutMs());
    lv_obj_add_event_cb(values->publishTimeoutInput, PublishTimeoutInputEventHandler, LV_EVENT_CLICKED, NULL);

    values->keyboard = lv_keyboard_create(GetPageBackground());
    lv_obj_set_size(values->keyboard, lv_display_get_horizontal_resolution(NULL), 160);
    lv_obj_align(values->keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(values->keyboard, KeyboardEventHandler, LV_EVENT_ALL, values);

    values->unsavedLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->unsavedLabel, "* Unsaved");
    lv_obj_align(values->unsavedLabel, LV_ALIGN_TOP_MID, 0, 8);

    UpdateAvatarPreview();
    SetUnsavedState(false);
}

static void MqttChatSettingsPageDeinit(void)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void MqttChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(code);
    UNUSED(data);
    UNUSED(dataLen);
}

static void BackButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (code == LV_EVENT_CLICKED) {
        if (values->unsaved) {
            ConfirmWin_t confirmWin = {0};
            confirmWin.text = "You have unsaved changes. Are you sure to leave without saving?";
            confirmWin.OkHandler = ConfirmBackEventHandler;
            CreateConfirmWin(GetPageBackground(), &confirmWin);
        } else {
            EnterPreviousPage();
        }
    }
}

static void ConfirmBackEventHandler(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        EnterPreviousPage();
    }
}

static void SaveBtnEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    UNUSED(e);

    if (!values->unsaved) {
        return;
    }

    DeviceSettingsSetMqttChatAvatarColor(GetSelectedAvatarColorValue(values->avatarColorDropdown));
    DeviceSettingsSetMqttChatUsername(lv_textarea_get_text(values->usernameInput));
    DeviceSettingsSetMqttBrokerHost(lv_textarea_get_text(values->brokerHostInput));
    DeviceSettingsSetMqttBrokerPort(strtoul(lv_textarea_get_text(values->brokerPortInput), NULL, 10));
    DeviceSettingsSetMqttTlsMode(strtoul(lv_textarea_get_text(values->tlsModeInput), NULL, 10));
    DeviceSettingsSetMqttClientIdPrefix(lv_textarea_get_text(values->clientIdPrefixInput));
    DeviceSettingsSetMqttAuthPrefix(lv_textarea_get_text(values->authPrefixInput));
    DeviceSettingsSetMqttSubscribeTopic(lv_textarea_get_text(values->subscribeTopicInput));
    DeviceSettingsSetMqttSubscribeQos(strtoul(lv_textarea_get_text(values->subscribeQosInput), NULL, 10));
    DeviceSettingsSetMqttPublishTimeoutMs(strtoul(lv_textarea_get_text(values->publishTimeoutInput), NULL, 10));
    SaveDeviceSettings();
    SetUnsavedState(false);
}

static void AvatarColorDropdownEventHandler(lv_event_t *e)
{
    UNUSED(e);
    UpdateAvatarPreview();
    UpdateUnsavedState();
}

static void TextInputEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(values->keyboard, target);
        lv_obj_remove_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (target == values->usernameInput) {
            UpdateAvatarPreview();
        }
        UpdateUnsavedState();
    }
}

static void KeyboardEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    MqttChatSettingsPageValues_t *values = lv_event_get_user_data(e);
    lv_obj_t *ta;

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        ta = lv_keyboard_get_textarea(values->keyboard);
        if (ta != NULL) {
            lv_obj_clear_state(ta, LV_STATE_FOCUSED);
            lv_indev_reset(NULL, ta);
        }
        lv_keyboard_set_textarea(values->keyboard, NULL);
        lv_obj_add_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
        UpdateUnsavedState();
    }
}

static void BrokerPortInputEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t port = strtoul(lv_textarea_get_text(values->brokerPortInput), NULL, 10);

    UNUSED(e);
    CreateInputNumber(GetPageBackground(), "MQTT Port", port, 1, 65535, BrokerPortInputNumberHandler);
}

static void BrokerPortInputNumberHandler(uint32_t input)
{
    char string[12];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->brokerPortInput, string);
    UpdateUnsavedState();
}

static void TlsModeInputEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t tlsMode = strtoul(lv_textarea_get_text(values->tlsModeInput), NULL, 10);

    UNUSED(e);
    CreateInputNumber(GetPageBackground(), "MQTT TLS Mode", tlsMode, 0, 9, TlsModeInputNumberHandler);
}

static void TlsModeInputNumberHandler(uint32_t input)
{
    char string[12];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->tlsModeInput, string);
    UpdateUnsavedState();
}

static void SubscribeQosInputEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t qos = strtoul(lv_textarea_get_text(values->subscribeQosInput), NULL, 10);

    UNUSED(e);
    CreateInputNumber(GetPageBackground(), "MQTT QoS", qos, 0, 2, SubscribeQosInputNumberHandler);
}

static void SubscribeQosInputNumberHandler(uint32_t input)
{
    char string[12];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->subscribeQosInput, string);
    UpdateUnsavedState();
}

static void PublishTimeoutInputEventHandler(lv_event_t *e)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t timeout = strtoul(lv_textarea_get_text(values->publishTimeoutInput), NULL, 10);

    UNUSED(e);
    CreateInputNumber(GetPageBackground(), "MQTT Publish Timeout", timeout, 1000, 60000, PublishTimeoutInputNumberHandler);
}

static void PublishTimeoutInputNumberHandler(uint32_t input)
{
    char string[12];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->publishTimeoutInput, string);
    UpdateUnsavedState();
}

static lv_obj_t *CreateRowLabel(const char *text, lv_coord_t y)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *label = lv_label_create(values->contentObj);
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, y);
    return label;
}

static lv_obj_t *CreateTextInput(lv_coord_t y, uint32_t maxLength, const char *text)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *input = lv_textarea_create(values->contentObj);

    lv_textarea_set_one_line(input, true);
    lv_textarea_set_max_length(input, maxLength);
    lv_obj_set_style_pad_all(input, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(input), lv_color_black(), 0);
    lv_obj_set_size(input, 148, 32);
    lv_obj_align(input, LV_ALIGN_TOP_RIGHT, -20, y - 6);
    lv_textarea_set_text(input, text);
    lv_obj_add_event_cb(input, TextInputEventHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(input, TextInputEventHandler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(input, TextInputEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    return input;
}

static lv_obj_t *CreateNumberInput(lv_coord_t y, uint32_t value)
{
    char string[12];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *input = lv_textarea_create(values->contentObj);

    snprintf(string, sizeof(string), "%lu", value);
    lv_textarea_set_one_line(input, true);
    lv_obj_set_style_pad_all(input, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(input), lv_color_black(), 0);
    lv_obj_set_size(input, 148, 32);
    lv_obj_align(input, LV_ALIGN_TOP_RIGHT, -20, y - 6);
    lv_textarea_set_text(input, string);

    return input;
}

static uint32_t GetSelectedAvatarColorValue(const lv_obj_t *dropdown)
{
    uint32_t index = lv_dropdown_get_selected(dropdown);

    if (index >= sizeof(g_avatarColorValues) / sizeof(g_avatarColorValues[0])) {
        return g_avatarColorValues[0];
    }

    return g_avatarColorValues[index];
}

static uint32_t GetAvatarColorSelectedIndex(uint32_t colorValue)
{
    for (uint32_t i = 0; i < sizeof(g_avatarColorValues) / sizeof(g_avatarColorValues[0]); i++) {
        if (g_avatarColorValues[i] == colorValue) {
            return i;
        }
    }

    return 0;
}

static void UpdateAvatarPreview(void)
{
    char avatarText[2];
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    const char *username = lv_textarea_get_text(values->usernameInput);
    uint32_t avatarColor = GetSelectedAvatarColorValue(values->avatarColorDropdown);

    avatarText[0] = (username[0] != '\0') ? username[0] : '?';
    avatarText[1] = '\0';
    lv_obj_set_style_bg_color(values->avatarPreview, lv_color_hex(avatarColor), 0);
    lv_label_set_text(values->avatarPreviewLabel, avatarText);
}

static void UpdateUnsavedState(void)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t avatarColor = GetSelectedAvatarColorValue(values->avatarColorDropdown);
    uint32_t brokerPort = strtoul(lv_textarea_get_text(values->brokerPortInput), NULL, 10);
    uint32_t tlsMode = strtoul(lv_textarea_get_text(values->tlsModeInput), NULL, 10);
    uint32_t subscribeQos = strtoul(lv_textarea_get_text(values->subscribeQosInput), NULL, 10);
    uint32_t publishTimeout = strtoul(lv_textarea_get_text(values->publishTimeoutInput), NULL, 10);
    bool unsaved = false;

    if (avatarColor != DeviceSettingsGetMqttChatAvatarColor()) {
        unsaved = true;
    }
    if (strcmp(lv_textarea_get_text(values->usernameInput), DeviceSettingsGetMqttChatUsername()) != 0) {
        unsaved = true;
    }
    if (strcmp(lv_textarea_get_text(values->brokerHostInput), DeviceSettingsGetMqttBrokerHost()) != 0) {
        unsaved = true;
    }
    if (brokerPort != DeviceSettingsGetMqttBrokerPort()) {
        unsaved = true;
    }
    if (tlsMode != DeviceSettingsGetMqttTlsMode()) {
        unsaved = true;
    }
    if (strcmp(lv_textarea_get_text(values->clientIdPrefixInput), DeviceSettingsGetMqttClientIdPrefix()) != 0) {
        unsaved = true;
    }
    if (strcmp(lv_textarea_get_text(values->authPrefixInput), DeviceSettingsGetMqttAuthPrefix()) != 0) {
        unsaved = true;
    }
    if (strcmp(lv_textarea_get_text(values->subscribeTopicInput), DeviceSettingsGetMqttSubscribeTopic()) != 0) {
        unsaved = true;
    }
    if (subscribeQos != DeviceSettingsGetMqttSubscribeQos()) {
        unsaved = true;
    }
    if (publishTimeout != DeviceSettingsGetMqttPublishTimeoutMs()) {
        unsaved = true;
    }

    SetUnsavedState(unsaved);
}

static void SetUnsavedState(bool unsaved)
{
    MqttChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    values->unsaved = unsaved;
    if (unsaved) {
        lv_obj_remove_flag(values->unsavedLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(values->unsavedLabel, LV_OBJ_FLAG_HIDDEN);
    }
}
