#include "status_bar.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "user_assert.h"
#include "user_utils.h"
#include "cmsis_os2.h"
#include "device_settings.h"
#include "images_declare.h"
#include "battery.h"
#include "sht30_app.h"

static void BatteryPadTimerFunc(lv_timer_t *timer);
static const lv_image_dsc_t *GetWifiSignalImageByRssi(int8_t rssi);

static lv_obj_t *g_statusBar;
static lv_obj_t *g_batteryImage;
static lv_obj_t *g_batteryPad;
static lv_obj_t *g_wifiImage;
static lv_obj_t *g_loraSignalImage;
static lv_obj_t *g_tempHumLabel;

static lv_timer_t *g_batteryPadTimer;


void CreateStatusBar(void)
{
    g_statusBar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_statusBar, lv_display_get_horizontal_resolution(NULL), STATUS_BAR_HEIGHT);

    g_batteryImage = lv_image_create(g_statusBar);
    lv_image_set_src(g_batteryImage, &img_battery);
    lv_obj_align(g_batteryImage, LV_ALIGN_RIGHT_MID, -10, 0);
    g_batteryPad = lv_obj_create(g_batteryImage);
    lv_obj_align(g_batteryPad, LV_ALIGN_TOP_LEFT, 4, 4);
    lv_obj_set_size(g_batteryPad, 0, 10);
    lv_obj_set_style_radius(g_batteryPad, 2, LV_PART_MAIN);
    lv_obj_clear_flag(g_batteryPad, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0xFF, 0xFF, 0xFF), 0);

    g_wifiImage = lv_image_create(g_statusBar);
    lv_image_set_src(g_wifiImage, &img_wifi_disabled);
    lv_obj_align(g_wifiImage, LV_ALIGN_RIGHT_MID, -50, 0);

    g_loraSignalImage = lv_image_create(g_statusBar);
    lv_image_set_src(g_loraSignalImage, &img_lora_signal);
    lv_obj_align(g_loraSignalImage, LV_ALIGN_RIGHT_MID, -79, 0);

    g_tempHumLabel = lv_label_create(g_statusBar);
    lv_label_set_text(g_tempHumLabel, "--.-C --.-%");
    lv_obj_set_style_text_color(g_tempHumLabel, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_align(g_tempHumLabel, LV_ALIGN_LEFT_MID, 10, 0);
}

void HandleStatusBarMsg(uint32_t code, void *data, uint32_t dataLen)
{
    uint32_t percent;
    BatteryStatus batteryStatus;
    Sht30Data_t sht30Data;
    UiMsgWifiStatus_t wifiStatus;

    switch (code) {
    case UI_MSG_CODE_BATTERY_PERCENT:
        ASSERT(data != NULL);
        ASSERT(dataLen == sizeof(percent));
        percent = *(uint8_t *)data;
        if (percent > 100) {
            percent = 100;
        }
        lv_obj_set_size(g_batteryPad, percent * 2 / 10, 10);
        break;
    case UI_MSG_CODE_CHARGING:
        ASSERT(data != NULL);
        ASSERT(dataLen == sizeof(batteryStatus));
        batteryStatus = *(BatteryStatus *)data;
        if (batteryStatus == BATTERY_CHARGING) {
            lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0x11, 0xEE, 0x11), 0);
            if (g_batteryPadTimer == NULL) {
                g_batteryPadTimer = lv_timer_create(BatteryPadTimerFunc, 500, NULL);
            }
        } else {
            if (batteryStatus == BATTERY_CHARGE_COMPLETE) {
                lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0x11, 0xEE, 0x11), 0);
            } else {
                lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0xFF, 0xFF, 0xFF), 0);
            }
            if (g_batteryPadTimer) {
                lv_timer_delete(g_batteryPadTimer);
                g_batteryPadTimer = NULL;
            }
        }
        break;
    case UI_MSG_CODE_SHT30:
        ASSERT(data != NULL);
        ASSERT(dataLen == sizeof(sht30Data));
        sht30Data = *(Sht30Data_t *)data;
        lv_label_set_text_fmt(g_tempHumLabel, "%d.%dC %u.%u%%",
                              sht30Data.temperatureCx10 / 10,
                              sht30Data.temperatureCx10 >= 0
                              ? sht30Data.temperatureCx10 % 10
                              : -(sht30Data.temperatureCx10 % 10),
                              sht30Data.humidityRhx10 / 10,
                              sht30Data.humidityRhx10 % 10);
        break;
    case UI_MSG_CODE_WIFI:
        ASSERT(data != NULL);
        ASSERT(dataLen == sizeof(wifiStatus));
        wifiStatus = *(UiMsgWifiStatus_t *)data;
        if (!wifiStatus.connected) {
            lv_image_set_src(g_wifiImage, &img_wifi_disabled);
        } else {
            lv_image_set_src(g_wifiImage, GetWifiSignalImageByRssi(wifiStatus.rssi));
        }
        break;
    default:
        break;
    }
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

static void BatteryPadTimerFunc(lv_timer_t *timer)
{
    static uint32_t padSize = 4;
    UNUSED(timer);

    padSize += 4;
    if (padSize > 20) {
        padSize = 4;
    }
    lv_obj_set_size(g_batteryPad, padSize, 10);
}

