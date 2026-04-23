#include "status_bar.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "user_assert.h"
#include "user_utils.h"
#include "cmsis_os2.h"
#include "device_settings.h"
#include "images_declare.h"

static void CommLedOffTimerFunc(lv_timer_t *timer);

static lv_obj_t *g_statusBar;
static lv_obj_t *g_commLed;
static lv_obj_t *g_batteryImage;
static lv_obj_t *g_batteryPad;
static lv_obj_t *g_wifiImage;
static lv_obj_t *g_loraSignalImage;

static lv_timer_t *g_commLedOffTimer;


void CreateStatusBar(void)
{
    g_statusBar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_statusBar, lv_display_get_horizontal_resolution(NULL), STATUS_BAR_HEIGHT);

    g_commLed = lv_led_create(g_statusBar);
    lv_obj_align(g_commLed, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_size(g_commLed, 5, 5);
    lv_led_set_color(g_commLed, lv_color_make(0, 0, 255));
    lv_led_off(g_commLed);

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
}

void HandleStatusBarMsg(uint32_t code, void *data, uint32_t dataLen)
{
    uint8_t percent;
    bool charging;

    switch (code) {
    case UI_MSG_CODE_COMM: {
        lv_led_set_color(g_commLed, lv_color_make(0xFF, 0x8C, 0x00));
        lv_led_on(g_commLed);
        g_commLedOffTimer = lv_timer_create(CommLedOffTimerFunc, 100, NULL);
    }
    break;
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
        ASSERT(dataLen == sizeof(charging));
        charging = *(bool *)data;
        if (charging) {
            lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0x11, 0xEE, 0x11), 0);
        } else {
            lv_obj_set_style_bg_color(g_batteryPad, lv_color_make(0xFF, 0xFF, 0xFF), 0);
        }
        break;
    default:
        break;
    }
}

static void CommLedOffTimerFunc(lv_timer_t *timer)
{
    UNUSED(timer);
    lv_led_off(g_commLed);
    lv_timer_delete(timer);
}

