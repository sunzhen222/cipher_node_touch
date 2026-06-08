#include "page.h"
#include "stdio.h"
#include "string.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "drv_lcd.h"

typedef struct {
    uint32_t brightness;
    uint32_t lockScreenTime;
    bool touchWakeupEnabled;
    bool valueChanged;
} LcdSettingsPageValues_t;


static void LcdSettingsPageInit(void);
static void LcdSettingsPageDeinit(void);
static void LcdSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

static void SliderBrightnessEventCallback(lv_event_t *e);
static void LockScreenTimeDropdownEventCallback(lv_event_t *e);
static void TouchWakeupSwitchEventCallback(lv_event_t *e);
static uint32_t GetLockScreenTimeSelectedIndex(uint32_t lockScreenTime);
static uint32_t GetSelectedLockScreenTimeValue(const lv_obj_t *dropdown);

static const char *g_lockScreenTimeDropdownOptions = "1 min\n2 min\n5 min\n10 min\n60 min\nNever";
static const uint32_t g_lockScreenTimeValues[] = {
    60,
    120,
    300,
    600,
    3600,
    3601,
};

Page_t g_lcdSettingsPage = {
    .init = LcdSettingsPageInit,
    .deinit = LcdSettingsPageDeinit,
    .msgHandler = LcdSettingsPageMsgHandler,
    .fullScreen = false,
};

static void LcdSettingsPageInit(void)
{
    LcdSettingsPageValues_t *values = SRAM_MALLOC(sizeof(LcdSettingsPageValues_t));
    memset(values, 0, sizeof(LcdSettingsPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);
    CreateGeneralNavigationBar();

    lv_obj_t *temp;

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "LCD Settings");
    lv_obj_align(temp, LV_ALIGN_TOP_MID, 0, 60);

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "Brightness");
    lv_obj_align(temp, LV_ALIGN_TOP_LEFT, 32, 118);

    temp = lv_slider_create(GetPageBackground());
    lv_slider_set_min_value(temp, 10);
    lv_slider_set_max_value(temp, 100);
    lv_slider_set_value(temp, DeviceSettingsGetBrightness(), LV_ANIM_OFF);
    lv_obj_set_width(temp, 272);
    lv_obj_set_height(temp, 20);
    lv_obj_align(temp, LV_ALIGN_TOP_MID, 0, 140);
    lv_obj_add_event_cb(temp, SliderBrightnessEventCallback, LV_EVENT_VALUE_CHANGED, NULL);

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "Lock Screen Time");
    lv_obj_align(temp, LV_ALIGN_TOP_LEFT, 32, 205);

    temp = lv_dropdown_create(GetPageBackground());
    lv_obj_set_size(temp, 120, 32);
    lv_obj_align(temp, LV_ALIGN_TOP_RIGHT, -32, 198);
    lv_dropdown_set_options(temp, g_lockScreenTimeDropdownOptions);
    lv_dropdown_set_selected(temp, GetLockScreenTimeSelectedIndex(DeviceSettingsGetLockScreenTime()));
    lv_obj_add_event_cb(temp, LockScreenTimeDropdownEventCallback, LV_EVENT_VALUE_CHANGED, NULL);

    temp = lv_label_create(GetPageBackground());
    lv_label_set_text(temp, "Touch Wakeup");
    lv_obj_align(temp, LV_ALIGN_TOP_LEFT, 32, 260);

    temp = lv_switch_create(GetPageBackground());
    lv_obj_set_size(temp, 64, 32);
    lv_obj_align(temp, LV_ALIGN_TOP_RIGHT, -32, 254);
    if (DeviceSettingsGetTouchWakeupEnabled()) {
        lv_obj_add_state(temp, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(temp, TouchWakeupSwitchEventCallback, LV_EVENT_VALUE_CHANGED, NULL);
}

static void LcdSettingsPageDeinit(void)
{
    LcdSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values->valueChanged) {
        SaveDeviceSettings();
    }
    SRAM_FREE(values);
}

static void LcdSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void SliderBrightnessEventCallback(lv_event_t *e)
{
    LcdSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *slider = lv_event_get_target(e);
    values->brightness = lv_slider_get_value(slider);
    printf("LCD Brightness: %lu\n", values->brightness);

    SetLcdBackLight(values->brightness);
    DeviceSettingsSetBrightness(values->brightness);
    values->valueChanged = true;
}

static void LockScreenTimeDropdownEventCallback(lv_event_t *e)
{
    LcdSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *dropdown = lv_event_get_target(e);
    values->lockScreenTime = GetSelectedLockScreenTimeValue(dropdown);
    printf("Lock Screen Time: %lu\n", values->lockScreenTime);

    DeviceSettingsSetLockScreenTime(values->lockScreenTime);
    values->valueChanged = true;
}

static void TouchWakeupSwitchEventCallback(lv_event_t *e)
{
    LcdSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    lv_obj_t *sw = lv_event_get_target(e);
    values->touchWakeupEnabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    printf("Touch Wakeup Enabled: %u\n", values->touchWakeupEnabled);

    DeviceSettingsSetTouchWakeupEnabled(values->touchWakeupEnabled);
    values->valueChanged = true;
}

static uint32_t GetLockScreenTimeSelectedIndex(uint32_t lockScreenTime)
{
    uint32_t i;

    if (lockScreenTime > 3600) {
        return 5;
    }

    for (i = 0; i < sizeof(g_lockScreenTimeValues) / sizeof(g_lockScreenTimeValues[0]); i++) {
        if (g_lockScreenTimeValues[i] == lockScreenTime) {
            return i;
        }
    }

    return 0;
}

static uint32_t GetSelectedLockScreenTimeValue(const lv_obj_t *dropdown)
{
    uint32_t index = lv_dropdown_get_selected(dropdown);
    if (index >= sizeof(g_lockScreenTimeValues) / sizeof(g_lockScreenTimeValues[0])) {
        index = 0;
    }

    return g_lockScreenTimeValues[index];
}
