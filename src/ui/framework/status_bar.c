#include "status_bar.h"
#include "stdio.h"
#include "lvgl.h"
#include "ui_msg.h"
#include "user_assert.h"
#include "user_utils.h"
#include "cmsis_os2.h"
#include "device_settings.h"

static void CommLedOffTimerFunc(lv_timer_t *timer);

static lv_obj_t *g_statusBar;
static lv_obj_t *g_commLed;

static lv_timer_t *g_commLedOffTimer;


void CreateStatusBar(void)
{
    g_statusBar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_statusBar, lv_display_get_horizontal_resolution(NULL), STATUS_BAR_HEIGHT);

    g_commLed = lv_led_create(g_statusBar);
    lv_obj_align(g_commLed, LV_ALIGN_LEFT_MID, 230, 0);
    lv_obj_set_size(g_commLed, 5, 5);
    lv_led_set_color(g_commLed, lv_color_make(0, 0, 255));
    lv_led_off(g_commLed);
}

void HandleStatusBarMsg(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    switch (code) {
    case UI_MSG_CODE_COMM: {
        lv_led_set_color(g_commLed, lv_color_make(0xFF, 0x8C, 0x00));
        lv_led_on(g_commLed);
        g_commLedOffTimer = lv_timer_create(CommLedOffTimerFunc, 100, NULL);
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

