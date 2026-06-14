#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "lora_chat.h"
#include "command_lora_chat.h"
#include "device_settings.h"
#include "status_bar.h"
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
    lv_timer_t *repeatSendTimer;
    uint32_t sendBtnRepeatCount;
} LoraChatPageValues_t;


static void LoraChatPageInit(void);
static void LoraChatPageDeinit(void);
static void LoraChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void LoraChatLayout(void);
static void AddNewLoraChatLayout(LoraChatItem_t *item);
static void InputTaEventHandler(lv_event_t *e);
static void InputKeyboardEventHandler(lv_event_t *e);
static void ChatListEventHandler(lv_event_t *e);
static void ScrollChatListToBottom(lv_obj_t *chatList);
static void InputSendBtnEventHandler(lv_event_t *e);
static void RepeatSendTimerHandler(lv_timer_t *timer);
static void StopRepeatSendTimer(LoraChatPageValues_t *values);
static bool SendCurrentLoraChatInput(LoraChatPageValues_t *values, bool clearInput);
static void UpdateSendButtonState(LoraChatPageValues_t *values);
static void HideInputKeyboardAndRestoreLayout(LoraChatPageValues_t *values);
static void ChatSettingsButtonHandler(lv_event_t *e);
static void BackButtonHandler(lv_event_t *e);


Page_t g_loraChatPage = {
    .init = LoraChatPageInit,
    .deinit = LoraChatPageDeinit,
    .msgHandler = LoraChatPageMsgHandler,
    .fullScreen = false,
};

static void LoraChatPageInit(void)
{
    NavigationBar_t navigationBar = {
        .leftImgSrc = &img_back,
        .leftBtnCb = BackButtonHandler,
        .rightImgSrc = &img_settings,
        .rightBtnCb = ChatSettingsButtonHandler,
        .middleText = "Lora Chat",
    };
    CreateNavigationBar(&navigationBar);
    LoraChatPageValues_t *values = SRAM_MALLOC(sizeof(LoraChatPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    lv_obj_set_scrollbar_mode(GetPageBackground(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLLABLE);
    values->chatList = NULL;
    values->inputBar = NULL;
    values->inputTa = NULL;
    values->keyboard = NULL;
    values->sendBtn = NULL;
    values->repeatSendTimer = NULL;
    values->sendBtnRepeatCount = 0;

    LoraChatLayout();
}

static void LoraChatPageDeinit(void)
{
    LoraChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    StopRepeatSendTimer(values);
    SRAM_FREE(values);
}

static void LoraChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    LoraChatItem_t *item;
    switch (code) {
    case UI_MSG_CODE_LORA_CHAT_ITEM:
        if (dataLen != sizeof(LoraChatItem_t *)) {
            printf("LoraChatPageMsgHandler: invalid dataLen\n");
            break;
        }
        item = *(LoraChatItem_t **)data;
        AddNewLoraChatLayout(item);
        break;
    default:
        break;
    }
}

static void LoraChatLayout(void)
{
    LoraChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

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
    lv_obj_set_size(values->chatList, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->chatList, LV_ALIGN_TOP_LEFT, 0, NAVIGATION_BAR_HEIGHT);
    lv_obj_set_scrollbar_mode(values->chatList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(values->chatList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(values->chatList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(values->chatList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_PRESSED, values);
    lv_obj_add_event_cb(values->chatList, ChatListEventHandler, LV_EVENT_CLICKED, values);

    StartGetChatItem();
    LoraChatItem_t *item;
    while ((item = GetNextChatItem()) != NULL) {
        AddNewLoraChatLayout(item);
    }

    values->inputBar = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->inputBar, lv_display_get_horizontal_resolution(NULL), INPUT_BAR_HEIGHT);
    lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(values->inputBar, lv_color_hex(0x222222), 0);
    //lv_obj_align_to(values->inputBar, values->chatList, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_obj_set_style_pad_left(values->inputBar, 4, 0);
    lv_obj_set_style_pad_right(values->inputBar, 4, 0);
    lv_obj_set_style_pad_top(values->inputBar, 4, 0);
    lv_obj_set_style_pad_bottom(values->inputBar, 4, 0);
    lv_obj_set_style_border_width(values->inputBar, 0, 0);

    values->inputTa = lv_textarea_create(values->inputBar);
    lv_obj_set_size(values->inputTa, lv_display_get_horizontal_resolution(NULL) - SEND_BTN_WIDTH - INPUT_BAR_GAP * 3, INPUT_BAR_HEIGHT - 8);
    lv_obj_align(values->inputTa, LV_ALIGN_LEFT_MID, 0, 0);
    lv_textarea_set_one_line(values->inputTa, true);
    lv_obj_set_style_pad_all(values->inputTa, 8, 0);
    lv_textarea_set_placeholder_text(values->inputTa, "Input message");
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
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(DeviceSettingsGetLoraChatAvatarColor()), 0);
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(0x134C26), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(values->sendBtn, lv_color_hex(0x4A4A4A), LV_STATE_DISABLED);
    lv_obj_set_style_text_color(values->sendBtn, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(values->sendBtn, lv_color_hex(0x9A9A9A), LV_STATE_DISABLED);
    lv_obj_add_event_cb(values->sendBtn, InputSendBtnEventHandler, LV_EVENT_SHORT_CLICKED, values);
    lv_obj_add_event_cb(values->sendBtn, InputSendBtnEventHandler, LV_EVENT_LONG_PRESSED_REPEAT, values);
    lv_obj_add_event_cb(values->sendBtn, InputSendBtnEventHandler, LV_EVENT_RELEASED, values);

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
    LoraChatPageValues_t *values = lv_event_get_user_data(e);

    if (code == LV_EVENT_FOCUSED || code == LV_EVENT_CLICKED) {
        lv_obj_set_size(values->chatList, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT - KEYBOARD_HEIGHT);
        lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, -KEYBOARD_HEIGHT);
        lv_keyboard_set_textarea(values->keyboard, values->inputTa);
        lv_obj_remove_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
        ScrollChatListToBottom(values->chatList);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        UpdateSendButtonState(values);
    }
}

static void InputKeyboardEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    LoraChatPageValues_t *values = lv_event_get_user_data(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_clear_state(values->inputTa, LV_STATE_FOCUSED);
        lv_indev_reset(NULL, values->inputTa);
        printf("Input text: %s\n", lv_textarea_get_text(values->inputTa));
        HideInputKeyboardAndRestoreLayout(values);
    }
}

static void ChatListEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    LoraChatPageValues_t *values = lv_event_get_user_data(e);

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

static void HideInputKeyboardAndRestoreLayout(LoraChatPageValues_t *values)
{
    if (values == NULL || values->keyboard == NULL || values->chatList == NULL || values->inputBar == NULL) {
        return;
    }

    lv_keyboard_set_textarea(values->keyboard, NULL);
    lv_obj_add_flag(values->keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(values->chatList, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT - INPUT_BAR_HEIGHT);
    lv_obj_align(values->inputBar, LV_ALIGN_BOTTOM_MID, 0, 0);
}

static void ScrollChatListToBottom(lv_obj_t *chatList)
{
    uint32_t childCount = lv_obj_get_child_count(chatList);
    if (childCount == 0) {
        return;
    }

    lv_obj_update_layout(chatList);
    lv_obj_t *lastItem = lv_obj_get_child(chatList, childCount - 1);
    if (lastItem != NULL) {
        lv_obj_scroll_to_view(lastItem, LV_ANIM_ON);
    }
}

static void AddNewLoraChatLayout(LoraChatItem_t *item)
{
    if (item == NULL) {
        return;
    }

    LoraChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values == NULL || values->chatList == NULL) {
        return;
    }

    lv_coord_t maxBubbleWidth = lv_display_get_horizontal_resolution(NULL) - 57 * 2;
    lv_coord_t maxTextWidth = maxBubbleWidth - 20;

    if (lv_obj_get_child_count(values->chatList) >= LORA_CHAT_MAX_ITEMS) {
        lv_obj_t *oldestRow = lv_obj_get_child(values->chatList, 0);
        if (oldestRow != NULL) {
            lv_obj_delete(oldestRow);
        }
    }

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

    ScrollChatListToBottom(values->chatList);
}

static void InputSendBtnEventHandler(lv_event_t *e)
{
    LoraChatPageValues_t *values = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED) {
        StopRepeatSendTimer(values);
        SendCurrentLoraChatInput(values, true);
        values->sendBtnRepeatCount = 0;
    } else if (code == LV_EVENT_LONG_PRESSED_REPEAT) {
        printf("long pressed repeat\n");
        values->sendBtnRepeatCount++;
        if (values->sendBtnRepeatCount >= 10) {
            values->sendBtnRepeatCount = 0;
            if (values->repeatSendTimer == NULL && lv_textarea_get_text(values->inputTa)[0] != '\0') {
                values->repeatSendTimer = lv_timer_create(RepeatSendTimerHandler, 1000, values);
            }
        }
    } else if (code == LV_EVENT_RELEASED) {
        values->sendBtnRepeatCount = 0;
    }

}

static void RepeatSendTimerHandler(lv_timer_t *timer)
{
    LoraChatPageValues_t *values = lv_timer_get_user_data(timer);
    if (!SendCurrentLoraChatInput(values, false)) {
        StopRepeatSendTimer(values);
    }
}

static void StopRepeatSendTimer(LoraChatPageValues_t *values)
{
    if (values == NULL || values->repeatSendTimer == NULL) {
        return;
    }

    lv_timer_delete(values->repeatSendTimer);
    values->repeatSendTimer = NULL;
}

static bool SendCurrentLoraChatInput(LoraChatPageValues_t *values, bool clearInput)
{
    if (values == NULL || values->inputTa == NULL) {
        return false;
    }

    const char *text = lv_textarea_get_text(values->inputTa);
    if (text[0] == '\0') {
        return false;
    }

    const char *username = DeviceSettingsGetLoraChatUsername();
    uint32_t avatarColor = DeviceSettingsGetLoraChatAvatarColor();

    printf("Send clicked, text: %s\n", text);
    LoraChatItem_t *newItem = AddChatItem(username, text, 0, true, avatarColor);
    SendLoraChat(username, text, avatarColor);
    SendUiMsg(UI_MSG_CODE_LORA_CHAT_ITEM, &newItem, sizeof(newItem));

    if (clearInput) {
        lv_textarea_set_text(values->inputTa, "");
        UpdateSendButtonState(values);
    }

    return true;
}

static void UpdateSendButtonState(LoraChatPageValues_t *values)
{
    const char *txt = lv_textarea_get_text(values->inputTa);
    if (txt[0] == '\0') {
        lv_obj_add_state(values->sendBtn, LV_STATE_DISABLED);
    } else {
        lv_obj_clear_state(values->sendBtn, LV_STATE_DISABLED);
    }
}

static void ChatSettingsButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        EnterNewPage(&g_loraChatSettingsPage);
    }
}

static void BackButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        EnterPreviousPage();
    }
}
