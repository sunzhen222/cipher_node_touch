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

typedef struct {
    lv_obj_t *listObj;
} WifiPageValues_t;

static void WifiPageInit(void);
static void WifiPageDeinit(void);
static void WifiPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

static void WifiSearchAndDisplay(void);
static const lv_image_dsc_t *GetWifiSignalImageByRssi(int8_t rssi);

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

    values->listObj = lv_obj_create(GetPageBackground());
    lv_obj_set_size(values->listObj, 300, 320);
    lv_obj_set_style_bg_color(values->listObj, lv_color_hex(0x202020), 0);
    lv_obj_align(values->listObj, LV_ALIGN_TOP_MID, 0, 122);
    lv_obj_set_style_radius(values->listObj, 12, 0);
    WifiSearchAndDisplay();
}

static void WifiPageDeinit(void)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}

static void WifiPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
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

static void WifiSearchAndDisplay(void)
{
    WifiPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    WiFiItem_t wifiHead = {0};
    uint32_t count = SearchWiFi(&wifiHead);

    printf("wifi scan count=%lu\n", count);
    lv_obj_t *label, *btn, *signalImg;

    WiFiItem_t *node = &wifiHead;
    for (uint32_t i = 0; i < count; i++) {
        ASSERT(node != NULL);
        printf("SSID: %s, CH: %u, Security: %s, RSSI: %d\n",
               node->ssid,
               node->ch,
               WiFiSecurityToString(node->security),
               node->rssi);
        btn = lv_button_create(values->listObj);
        lv_obj_set_size(btn, 300, 40);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, i * 40);
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
    }

    FreeWiFiList(&wifiHead);
}
