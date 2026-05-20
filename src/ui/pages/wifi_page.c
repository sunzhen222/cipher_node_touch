#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "search_wifi.h"
#include "images_declare.h"
#include "user_assert.h"
#include "background_task.h"

typedef struct {
    lv_obj_t *listObj;
    lv_obj_t *refreshBtn;
    lv_obj_t *refreshImage;
} WifiPageValues_t;

static void WifiPageInit(void);
static void WifiPageDeinit(void);
static void WifiPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void RefreshBtnClickHandler(lv_event_t *e);

static const lv_image_dsc_t *GetWifiSignalImageByRssi(int8_t rssi);
static void RefreshRotateAnimExecCb(void *var, int32_t value);
static void StartRefreshRotationAnim(lv_obj_t *refreshImage);
static void StopRefreshRotationAnim(lv_obj_t *refreshImage);
static int32_t AsyncWifiSearch(const void *inData, uint32_t inDataLen);
static void WifiSearchResultDisplay(WiFiItem_t *wifiListHead);

Page_t g_wifiPage = {
    .init = WifiPageInit,
    .deinit = WifiPageDeinit,
    .msgHandler = WifiPageMsgHandler,
    .fullScreen = false,
};

static void WifiPageInit(void)
{
    CreateGeneralNavigationBar();
    WifiPageValues_t *values = SRAM_MALLOC(sizeof(WifiPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    lv_obj_t *label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Connected Wi-Fi");
    lv_obj_set_style_text_color(label, lv_color_hex(0x888888), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 37);

    label = lv_label_create(GetPageBackground());
    lv_label_set_text(label, "Nearby Wi-Fi");
    lv_obj_set_style_text_color(label, lv_color_hex(0x888888), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 102);

    values->refreshBtn = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->refreshBtn, 50, 20);
    values->refreshImage = lv_image_create(values->refreshBtn);
    lv_image_set_src(values->refreshImage, LV_SYMBOL_REFRESH);
    lv_obj_align(values->refreshImage, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(values->refreshBtn, LV_ALIGN_TOP_RIGHT, -10, 100);
    lv_obj_set_style_radius(values->refreshBtn, 3, 0);
    lv_obj_set_style_bg_color(values->refreshBtn, lv_color_black(), 0);
    lv_obj_set_style_bg_color(values->refreshBtn, lv_color_hex(0x606060), LV_STATE_PRESSED);
    lv_obj_set_style_bg_color(values->refreshBtn, lv_color_black(), LV_STATE_DISABLED);
    lv_obj_add_event_cb(values->refreshBtn, RefreshBtnClickHandler, LV_EVENT_CLICKED, NULL);

    lv_obj_set_style_transform_height(values->refreshBtn, 0, LV_STATE_PRESSED);

    values->listObj = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->listObj, 300, 320);
    lv_obj_set_style_bg_color(values->listObj, lv_color_hex(0x202020), 0);
    lv_obj_align(values->listObj, LV_ALIGN_TOP_MID, 0, 122);
    lv_obj_set_style_radius(values->listObj, 12, 0);

    lv_obj_add_state(values->refreshBtn, LV_STATE_DISABLED);
    StartRefreshRotationAnim(values->refreshImage);
    AsyncExecute(AsyncWifiSearch, NULL, 0, 0);
}

static void WifiPageDeinit(void)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    StopRefreshRotationAnim(values->refreshImage);
    SRAM_FREE(values);
}

static void WifiPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    switch (code) {
    case UI_MSG_CODE_WIFI_SEARCH_RESULT:
        printf("received wifi search result msg:len=%lu\n", dataLen);
        StopRefreshRotationAnim(values->refreshImage);
        lv_obj_remove_state(values->refreshBtn, LV_STATE_DISABLED);
        if (dataLen == sizeof(WiFiItem_t)) {
            WiFiItem_t *wifiListHead = (WiFiItem_t *)(data);
            WifiSearchResultDisplay(wifiListHead);
            FreeWiFiList(wifiListHead);
        }
        break;
    default:
        break;
    }
}

static void RefreshBtnClickHandler(lv_event_t *e)
{
    UNUSED(e);

    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    ASSERT(values != NULL);

    lv_obj_clean(values->listObj);
    lv_obj_add_state(values->refreshBtn, LV_STATE_DISABLED);
    StartRefreshRotationAnim(values->refreshImage);
    AsyncExecute(AsyncWifiSearch, NULL, 0, 0);
}

static void StartRefreshRotationAnim(lv_obj_t *refreshImage)
{
    ASSERT(refreshImage != NULL);

    lv_obj_update_layout(refreshImage);
    lv_obj_set_style_transform_pivot_x(refreshImage, lv_obj_get_width(refreshImage) / 2, 0);
    lv_obj_set_style_transform_pivot_y(refreshImage, lv_obj_get_height(refreshImage) / 2, 0);
    lv_obj_set_style_transform_rotation(refreshImage, 0, 0);

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, refreshImage);
    lv_anim_set_exec_cb(&a, RefreshRotateAnimExecCb);
    lv_anim_set_values(&a, 0, 3600);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_start(&a);
}

static void StopRefreshRotationAnim(lv_obj_t *refreshImage)
{
    ASSERT(refreshImage != NULL);

    lv_anim_delete(refreshImage, RefreshRotateAnimExecCb);
    lv_obj_set_style_transform_rotation(refreshImage, 0, 0);
}

static void RefreshRotateAnimExecCb(void *var, int32_t value)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_style_transform_rotation(obj, value, 0);
}

static const lv_image_dsc_t *GetWifiSignalImageByRssi(int8_t rssi)
{
    if (rssi >= -50) {
        return &img_wifi_signal;
    }
    if (rssi >= -70) {
        return &img_wifi_signal_weak3;
    }
    if (rssi >= -90) {
        return &img_wifi_signal_weak2;
    }
    return &img_wifi_signal_weak1;
}

static int32_t AsyncWifiSearch(const void *inData, uint32_t inDataLen)
{
    UNUSED(inData);
    UNUSED(inDataLen);
    WiFiItem_t wifiHead = {0};
    uint32_t count = SearchWiFi(&wifiHead);
    printf("wifi scan count=%lu\n", count);
    SendUiMsg(UI_MSG_CODE_WIFI_SEARCH_RESULT, &wifiHead, sizeof(WiFiItem_t));
    return 0;
}

static void WifiSearchResultDisplay(WiFiItem_t *wifiListHead)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    WiFiItem_t *node = wifiListHead;
    lv_obj_t *label, *btn, *signalImg;
    uint32_t index = 0;

    while (node != NULL) {
        printf("SSID: %s, CH: %u, Security: %s, RSSI: %d\n",
               node->ssid,
               node->ch,
               WiFiSecurityToString(node->security),
               node->rssi);
        btn = lv_button_create(values->listObj);
        lv_obj_set_size(btn, 300, 40);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, index * 40);
        lv_obj_set_layout(btn, LV_LAYOUT_NONE);
        lv_obj_set_style_radius(btn, 12, 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x202020), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x606060), LV_STATE_PRESSED);
        lv_obj_set_style_transform_height(btn, 0, LV_STATE_PRESSED);

        signalImg = lv_image_create(btn);
        lv_image_set_src(signalImg, GetWifiSignalImageByRssi(node->rssi));
        lv_obj_add_flag(signalImg, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(signalImg, LV_ALIGN_LEFT_MID, 12, 0);
        label = lv_label_create(btn);
        lv_label_set_text(label, node->ssid);
        lv_obj_add_flag(label, LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 52, 0);

        node = node->next;
        index++;
    }
}