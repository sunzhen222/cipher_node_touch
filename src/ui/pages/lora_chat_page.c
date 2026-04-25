#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "lora_chat.h"
#include "status_bar.h"

typedef struct {
    lv_obj_t *chatList;
} LoraChatPageValues_t;


static void LoraChatPageInit(void);
static void LoraChatPageDeinit(void);
static void LoraChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void LoraChatLayout(void);


Page_t g_loraChatPage = {
    .init = LoraChatPageInit,
    .deinit = LoraChatPageDeinit,
    .msgHandler = LoraChatPageMsgHandler,
    .fullScreen = false,
};

static void LoraChatPageInit(void)
{
    CreateGeneralNavigationBar();
    LoraChatPageValues_t *values = SRAM_MALLOC(sizeof(LoraChatPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    lv_obj_set_scrollbar_mode(GetPageBackground(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(GetPageBackground(), LV_OBJ_FLAG_SCROLLABLE);
    values->chatList = NULL;

    TestLoraChat();
    LoraChatLayout();
}

static void LoraChatPageDeinit(void)
{
    LoraChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void LoraChatPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void LoraChatLayout(void)
{
    LoraChatPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (values->chatList != NULL) {
        lv_obj_delete(values->chatList);
        values->chatList = NULL;
    }

    values->chatList = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->chatList, lv_display_get_horizontal_resolution(NULL), lv_display_get_vertical_resolution(NULL) - STATUS_BAR_HEIGHT - NAVIGATION_BAR_HEIGHT);
    lv_obj_align(values->chatList, LV_ALIGN_TOP_LEFT, 0, NAVIGATION_BAR_HEIGHT);
    lv_obj_set_scrollbar_mode(values->chatList, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(values->chatList, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(values->chatList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(values->chatList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    StartGetChatItem();
    ChatItem_t *item;
    lv_coord_t maxBubbleWidth = (lv_display_get_horizontal_resolution(NULL) * 78) / 100;
    lv_coord_t maxTextWidth = maxBubbleWidth - 20;
    while ((item = GetNextChatItem()) != NULL) {

        lv_obj_t *row = lv_obj_create(values->chatList);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);

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

        //char rssiText[20];
        //lv_snprintf(rssiText, sizeof(rssiText), "RSSI %u", item->rssi);
        //lv_obj_t *rssiLabel = lv_label_create(bubble);
        //lv_label_set_text(rssiLabel, rssiText);
        //lv_obj_set_style_text_color(rssiLabel, lv_color_hex(0x7A7A7A), 0);
        //lv_obj_set_style_text_align(rssiLabel, LV_TEXT_ALIGN_RIGHT, 0);
        //lv_obj_set_width(rssiLabel, lv_pct(100));
    }
}
