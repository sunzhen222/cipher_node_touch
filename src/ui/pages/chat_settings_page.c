#include "page.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
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
    lv_obj_t *avatarColorDropdown;
    lv_obj_t *usernameInput;
    lv_obj_t *secretKeyInput;
    lv_obj_t *frequencyInput;
    lv_obj_t *netIdInput;
    lv_obj_t *sfDropdown;
    lv_obj_t *bwDropdown;
    lv_obj_t *usernameKeyboard;
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
static void NetIdInputEventHandler(lv_event_t *e);
static void NetIdInputNumberHandler(uint32_t input);
static void AvatarColorDropdownEventHandler(lv_event_t *e);
static void UsernameInputEventHandler(lv_event_t *e);
static void SecretKeyInputEventHandler(lv_event_t *e);
static void UsernameKeyboardEventHandler(lv_event_t *e);
static void SfDropdownEventHandler(lv_event_t *e);
static void BwDropdownEventHandler(lv_event_t *e);

static void SetUnsavedState(bool unsaved);
static uint32_t GetSelectedSfValue(const lv_obj_t *dropdown);
static uint32_t GetSelectedBwValue(const lv_obj_t *dropdown);
static uint32_t GetSelectedAvatarColorValue(const lv_obj_t *dropdown);
static uint32_t GetSfSelectedIndex(uint32_t sfValue);
static uint32_t GetBwSelectedIndex(uint32_t bwValue);
static uint32_t GetAvatarColorSelectedIndex(uint32_t colorValue);
static void UpdateUnsavedState(void);

static const char *g_avatarColorDropdownOptions = "Green\nBlue\nRed\nOrange\nPurple";
static const char *g_sfDropdownOptions = "SF5\nSF6\nSF7\nSF8\nSF9\nSF10\nSF11";
static const char *g_bwDropdownOptions = "125 kHz\n250 kHz\n500 kHz";
static const uint32_t g_avatarColorValues[] = {
    0x2FB35A,
    0x6155F5,
    0xFF2C2C,
    0xFF8A00,
    0x8A3FFC,
};
static const uint32_t g_bwValues[] = {
    LLCC68_LORA_BANDWIDTH_125_KHZ,
    LLCC68_LORA_BANDWIDTH_250_KHZ,
    LLCC68_LORA_BANDWIDTH_500_KHZ,
};

Page_t g_chatSettingsPage = {
    .init = ChatSettingsPageInit,
    .deinit = ChatSettingsPageDeinit,
    .msgHandler = ChatSettingsPageMsgHandler,
    .fullScreen = false,
};


static void ChatSettingsPageInit(void)
{
    char string[32];

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
    lv_label_set_text(label, "Avatar Color");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 56);
    values->avatarColorDropdown = lv_dropdown_create(GetPageBackground());
    lv_obj_set_size(values->avatarColorDropdown, 120, 32);
    lv_obj_align(values->avatarColorDropdown, LV_ALIGN_TOP_RIGHT, -32, 50);
    lv_dropdown_set_options(values->avatarColorDropdown, g_avatarColorDropdownOptions);
    lv_dropdown_set_selected(values->avatarColorDropdown, GetAvatarColorSelectedIndex(DeviceSettingsGetLoraChatAvatarColor()));
    lv_obj_add_event_cb(values->avatarColorDropdown, AvatarColorDropdownEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Username");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 100);
    values->usernameInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_one_line(values->usernameInput, true);
    lv_textarea_set_max_length(values->usernameInput, 31);
    lv_obj_set_style_pad_all(values->usernameInput, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->usernameInput), lv_color_black(), 0);
    lv_obj_set_size(values->usernameInput, 120, 32);
    lv_obj_align(values->usernameInput, LV_ALIGN_TOP_RIGHT, -32, 94);
    lv_textarea_set_text(values->usernameInput, DeviceSettingsGetLoraChatUsername());
    lv_obj_add_event_cb(values->usernameInput, UsernameInputEventHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(values->usernameInput, UsernameInputEventHandler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(values->usernameInput, UsernameInputEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "LoRa Secret Key");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 144);
    values->secretKeyInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_one_line(values->secretKeyInput, true);
    lv_textarea_set_max_length(values->secretKeyInput, 95);
    lv_obj_set_style_pad_all(values->secretKeyInput, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->secretKeyInput), lv_color_black(), 0);
    lv_obj_set_size(values->secretKeyInput, 120, 32);
    lv_obj_align(values->secretKeyInput, LV_ALIGN_TOP_RIGHT, -32, 138);
    lv_textarea_set_text(values->secretKeyInput, DeviceSettingsGetLoraSecretKey());
    lv_obj_add_event_cb(values->secretKeyInput, SecretKeyInputEventHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(values->secretKeyInput, SecretKeyInputEventHandler, LV_EVENT_FOCUSED, NULL);
    lv_obj_add_event_cb(values->secretKeyInput, SecretKeyInputEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Frequency");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 188);
    values->frequencyInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_one_line(values->frequencyInput, true);
    lv_obj_set_style_pad_all(values->frequencyInput, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->frequencyInput), lv_color_black(), 0);
    lv_obj_set_size(values->frequencyInput, 120, 32);
    lv_obj_align(values->frequencyInput, LV_ALIGN_TOP_RIGHT, -32, 182);
    snprintf(string, sizeof(string), "%lu", DeviceSettingsGetLoraFreq());
    lv_textarea_set_text(values->frequencyInput, string);
    lv_obj_add_event_cb(values->frequencyInput, FrequencyInputEventHandler, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "LoRa Net ID");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 232);
    values->netIdInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_one_line(values->netIdInput, true);
    lv_obj_set_style_pad_all(values->netIdInput, 8, 0);
    lv_obj_set_style_text_color(lv_textarea_get_label(values->netIdInput), lv_color_black(), 0);
    lv_obj_set_size(values->netIdInput, 120, 32);
    lv_obj_align(values->netIdInput, LV_ALIGN_TOP_RIGHT, -32, 226);
    snprintf(string, sizeof(string), "%lu", DeviceSettingsGetLoraNetId());
    lv_textarea_set_text(values->netIdInput, string);
    lv_obj_add_event_cb(values->netIdInput, NetIdInputEventHandler, LV_EVENT_CLICKED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Spreading Factor");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 276);
    values->sfDropdown = lv_dropdown_create(GetPageBackground());
    lv_obj_set_size(values->sfDropdown, 120, 32);
    lv_obj_align(values->sfDropdown, LV_ALIGN_TOP_RIGHT, -32, 270);
    lv_dropdown_set_options(values->sfDropdown, g_sfDropdownOptions);
    lv_dropdown_set_selected(values->sfDropdown, GetSfSelectedIndex(DeviceSettingsGetLoraSpreadingFactor()));
    lv_obj_add_event_cb(values->sfDropdown, SfDropdownEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Bandwidth");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 32, 320);
    values->bwDropdown = lv_dropdown_create(GetPageBackground());
    lv_obj_set_size(values->bwDropdown, 120, 32);
    lv_obj_align(values->bwDropdown, LV_ALIGN_TOP_RIGHT, -32, 314);
    lv_dropdown_set_options(values->bwDropdown, g_bwDropdownOptions);
    lv_dropdown_set_selected(values->bwDropdown, GetBwSelectedIndex(DeviceSettingsGetLoraBandwidth()));
    lv_obj_add_event_cb(values->bwDropdown, BwDropdownEventHandler, LV_EVENT_VALUE_CHANGED, NULL);

    values->usernameKeyboard = lv_keyboard_create(GetPageBackground());
    lv_obj_set_size(values->usernameKeyboard, lv_display_get_horizontal_resolution(NULL), 160);
    lv_obj_align(values->usernameKeyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(values->usernameKeyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(values->usernameKeyboard, UsernameKeyboardEventHandler, LV_EVENT_ALL, values);

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
    const char *username = lv_textarea_get_text(values->usernameInput);
    const char *secretKey = lv_textarea_get_text(values->secretKeyInput);
    uint32_t avatarColor = GetSelectedAvatarColorValue(values->avatarColorDropdown);
    uint32_t frequency = strtoul(lv_textarea_get_text(values->frequencyInput), NULL, 10);
    uint32_t netId = strtoul(lv_textarea_get_text(values->netIdInput), NULL, 10);
    uint32_t sf = GetSelectedSfValue(values->sfDropdown);
    uint32_t bw = GetSelectedBwValue(values->bwDropdown);
    bool needUpdateLora = false;

    UNUSED(e);

    if (!values->unsaved) {
        return;
    }
    printf("avatarColor: 0x%06lX\n", avatarColor);
    printf("username: %s\n", username);
    printf("secretKey: %s\n", secretKey);
    printf("frequency: %lu\n", frequency);
    printf("loraNetId: %lu\n", netId);
    printf("loraSf: %lu\n", sf);
    printf("loraBw: %lu\n", bw);
    if (frequency != DeviceSettingsGetLoraFreq() ||
            sf != DeviceSettingsGetLoraSpreadingFactor() ||
            bw != DeviceSettingsGetLoraBandwidth()) {
        needUpdateLora = true;
    }

    DeviceSettingsSetLoraChatAvatarColor(avatarColor);
    DeviceSettingsSetLoraChatUsername(username);
    DeviceSettingsSetLoraSecretKey(secretKey);
    DeviceSettingsSetLoraFreq(frequency);
    DeviceSettingsSetLoraNetId(netId);
    DeviceSettingsSetLoraSpreadingFactor(sf);
    DeviceSettingsSetLoraBandwidth(bw);
    SaveDeviceSettings();
    if (needUpdateLora) {
        LoraSettings();
    }
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
    UpdateUnsavedState();
}

static void NetIdInputEventHandler(lv_event_t *e)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    uint32_t netId = strtoul(lv_textarea_get_text(values->netIdInput), NULL, 10);

    UNUSED(e);

    CreateInputNumber(GetPageBackground(), "LoRa Net ID", netId, 0, 0xFFFFFFFFU, NetIdInputNumberHandler);
}

static void NetIdInputNumberHandler(uint32_t input)
{
    char string[12];
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    snprintf(string, sizeof(string), "%lu", input);
    lv_textarea_set_text(values->netIdInput, string);
    UpdateUnsavedState();
}

static void AvatarColorDropdownEventHandler(lv_event_t *e)
{
    UNUSED(e);
    UpdateUnsavedState();
}

static void UsernameInputEventHandler(lv_event_t *e)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(values->usernameKeyboard, values->usernameInput);
        lv_obj_remove_flag(values->usernameKeyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        UpdateUnsavedState();
    }
}

static void SecretKeyInputEventHandler(lv_event_t *e)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        lv_keyboard_set_textarea(values->usernameKeyboard, values->secretKeyInput);
        lv_obj_remove_flag(values->usernameKeyboard, LV_OBJ_FLAG_HIDDEN);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        UpdateUnsavedState();
    }
}

static void UsernameKeyboardEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    ChatSettingsPageValues_t *values = lv_event_get_user_data(e);
    lv_obj_t *ta;

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        ta = lv_keyboard_get_textarea(values->usernameKeyboard);
        if (ta != NULL) {
            lv_obj_clear_state(ta, LV_STATE_FOCUSED);
            lv_indev_reset(NULL, ta);
        }
        lv_keyboard_set_textarea(values->usernameKeyboard, NULL);
        lv_obj_add_flag(values->usernameKeyboard, LV_OBJ_FLAG_HIDDEN);
        UpdateUnsavedState();
    }
}

static void SfDropdownEventHandler(lv_event_t *e)
{
    UNUSED(e);
    UpdateUnsavedState();
}

static void BwDropdownEventHandler(lv_event_t *e)
{
    UNUSED(e);
    UpdateUnsavedState();
}

static uint32_t GetSelectedSfValue(const lv_obj_t *dropdown)
{
    return LLCC68_LORA_SF_5 + lv_dropdown_get_selected(dropdown);
}

static uint32_t GetSelectedBwValue(const lv_obj_t *dropdown)
{
    uint32_t index = lv_dropdown_get_selected(dropdown);

    if (index >= sizeof(g_bwValues) / sizeof(g_bwValues[0])) {
        return LLCC68_LORA_BANDWIDTH_250_KHZ;
    }

    return g_bwValues[index];
}

static uint32_t GetSelectedAvatarColorValue(const lv_obj_t *dropdown)
{
    uint32_t index = lv_dropdown_get_selected(dropdown);

    if (index >= sizeof(g_avatarColorValues) / sizeof(g_avatarColorValues[0])) {
        return g_avatarColorValues[0];
    }

    return g_avatarColorValues[index];
}

static uint32_t GetSfSelectedIndex(uint32_t sfValue)
{
    if (sfValue < LLCC68_LORA_SF_5 || sfValue > LLCC68_LORA_SF_11) {
        return LLCC68_LORA_SF_7 - LLCC68_LORA_SF_5;
    }

    return sfValue - LLCC68_LORA_SF_5;
}

static uint32_t GetBwSelectedIndex(uint32_t bwValue)
{
    for (uint32_t i = 0; i < sizeof(g_bwValues) / sizeof(g_bwValues[0]); i++) {
        if (g_bwValues[i] == bwValue) {
            return i;
        }
    }

    return 1;
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

static void UpdateUnsavedState(void)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    const char *username = lv_textarea_get_text(values->usernameInput);
    const char *secretKey = lv_textarea_get_text(values->secretKeyInput);
    uint32_t avatarColor = GetSelectedAvatarColorValue(values->avatarColorDropdown);
    uint32_t frequency = strtoul(lv_textarea_get_text(values->frequencyInput), NULL, 10);
    uint32_t netId = strtoul(lv_textarea_get_text(values->netIdInput), NULL, 10);
    uint32_t sf = GetSelectedSfValue(values->sfDropdown);
    uint32_t bw = GetSelectedBwValue(values->bwDropdown);

    bool unsaved = false;
    if (avatarColor != DeviceSettingsGetLoraChatAvatarColor()) {
        unsaved = true;
    }
    if (strcmp(username, DeviceSettingsGetLoraChatUsername()) != 0) {
        unsaved = true;
    }
    if (strcmp(secretKey, DeviceSettingsGetLoraSecretKey()) != 0) {
        unsaved = true;
    }
    if (frequency != DeviceSettingsGetLoraFreq()) {
        unsaved = true;
    }
    if (netId != DeviceSettingsGetLoraNetId()) {
        unsaved = true;
    }
    if (sf != DeviceSettingsGetLoraSpreadingFactor()) {
        unsaved = true;
    }
    if (bw != DeviceSettingsGetLoraBandwidth()) {
        unsaved = true;
    }
    SetUnsavedState(unsaved);
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