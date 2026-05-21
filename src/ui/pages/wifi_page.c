#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "wifi_search.h"
#include "wifi_connect.h"
#include "images_declare.h"
#include "user_assert.h"
#include "background_task.h"

typedef struct {
    lv_obj_t *connectedLabel;
    lv_obj_t *connectedButton;
    lv_obj_t *connectedSignalImg;
    lv_obj_t *connectedSsidLabel;
    lv_obj_t *connectedSecurityImg;
    lv_obj_t *nearbyLabel;
    lv_obj_t *refreshBtn;
    lv_obj_t *refreshImage;
    lv_obj_t *listObj;
    WifiConnectInfo_t connectInfo;
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
static void WifiSearchResultDisplay(WifiItem_t *wifiListHead);
static void WifiConnectStatusDisplay(void);
static void ConnectedLayout(bool connected);

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

    values->connectedLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->connectedLabel, "Connected Wi-Fi");
    lv_obj_set_style_text_color(values->connectedLabel, lv_color_hex(0x888888), 0);
    lv_obj_align(values->connectedLabel, LV_ALIGN_TOP_LEFT, 10, 37);

    values->connectedButton = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->connectedButton, 300, 40);
    lv_obj_align(values->connectedButton, LV_ALIGN_TOP_MID, 0, 57);
    lv_obj_set_layout(values->connectedButton, LV_LAYOUT_NONE);
    lv_obj_set_style_radius(values->connectedButton, 12, 0);
    values->connectedSignalImg = lv_image_create(values->connectedButton);
    lv_obj_align(values->connectedSignalImg, LV_ALIGN_LEFT_MID, 12, 0);
    values->connectedSsidLabel = lv_label_create(values->connectedButton);
    lv_label_set_long_mode(values->connectedSsidLabel, LV_LABEL_LONG_MODE_DOTS);
    lv_obj_align(values->connectedSsidLabel, LV_ALIGN_LEFT_MID, 52, 0);
    lv_obj_set_width(values->connectedSsidLabel, 200);
    values->connectedSecurityImg = lv_image_create(values->connectedButton);
    lv_obj_align(values->connectedSecurityImg, LV_ALIGN_RIGHT_MID, -12, 0);
    lv_image_set_src(values->connectedSecurityImg, &img_lock);

    values->nearbyLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->nearbyLabel, "Nearby Wi-Fi");
    lv_obj_set_style_text_color(values->nearbyLabel, lv_color_hex(0x888888), 0);
    lv_obj_align(values->nearbyLabel, LV_ALIGN_TOP_LEFT, 10, 102);

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

    GetWifiConnectInfo(&values->connectInfo);
    ConnectedLayout(values->connectInfo.connected);
    WifiConnectStatusDisplay();
    printf("wifi connected: %s, ssid: %s\n", values->connectInfo.connected ? "yes" : "no", values->connectInfo.ssid);

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
        if (dataLen == sizeof(WifiItem_t)) {
            WifiItem_t *wifiListHead = (WifiItem_t *)(data);
            WifiSearchResultDisplay(wifiListHead);
            FreeWifiList(wifiListHead);
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
    WifiItem_t wifiHead = {0};
    uint32_t count = SearchWifi(&wifiHead);
    printf("wifi scan count=%lu\n", count);
    SendUiMsg(UI_MSG_CODE_WIFI_SEARCH_RESULT, &wifiHead, sizeof(WifiItem_t));
    return 0;
}

static void WifiSearchResultDisplay(WifiItem_t *wifiListHead)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    WifiItem_t *node = wifiListHead;
    lv_obj_t *label, *btn, *signalImg, *lockImg;
    uint32_t index = 0;

    while (node != NULL) {
        printf("SSID: %s, CH: %u, Security: %s, RSSI: %d\n",
               node->ssid,
               node->ch,
               WifiSecurityToString(node->security),
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
        lv_obj_align(signalImg, LV_ALIGN_LEFT_MID, 12, 0);
        label = lv_label_create(btn);
        lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS);
        lv_label_set_text(label, node->ssid);
        lv_obj_align(label, LV_ALIGN_LEFT_MID, 52, 0);
        lv_obj_set_width(label, 200);
        if (node->security != WIFI_SECURITY_OPEN) {
            lockImg = lv_image_create(btn);
            lv_image_set_src(lockImg, &img_lock);
            lv_obj_align(lockImg, LV_ALIGN_RIGHT_MID, -12, 0);
        }

        node = node->next;
        index++;
    }
}

static void WifiConnectStatusDisplay(void)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (!values->connectInfo.connected) {
        return;
    }

    lv_label_set_text(values->connectedSsidLabel, values->connectInfo.ssid);

    if (values->connectInfo.security == WIFI_SECURITY_OPEN) {
        lv_obj_add_flag(values->connectedSecurityImg, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(values->connectedSecurityImg, LV_OBJ_FLAG_HIDDEN);
    }

    int8_t rssi = -127;
    if (GetWifiRssi(&rssi)) {
        lv_image_set_src(values->connectedSignalImg, GetWifiSignalImageByRssi(rssi));
    } else {
        lv_image_set_src(values->connectedSignalImg, &img_wifi_signal_weak1);
    }
}

static void ConnectedLayout(bool connected)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (connected) {
        lv_obj_remove_flag(values->connectedLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_remove_flag(values->connectedButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(values->nearbyLabel, LV_ALIGN_TOP_LEFT, 10, 102);
        lv_obj_align(values->refreshBtn, LV_ALIGN_TOP_RIGHT, -10, 100);
        lv_obj_set_size(values->listObj, 300, 320);
        lv_obj_align(values->listObj, LV_ALIGN_TOP_MID, 0, 122);
    } else {
        lv_obj_add_flag(values->connectedLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(values->connectedButton, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(values->nearbyLabel, LV_ALIGN_TOP_LEFT, 10, 37);
        lv_obj_align(values->refreshBtn, LV_ALIGN_TOP_RIGHT, -10, 35);
        lv_obj_set_size(values->listObj, 300, 385);
        lv_obj_align(values->listObj, LV_ALIGN_TOP_MID, 0, 57);
    }
}
