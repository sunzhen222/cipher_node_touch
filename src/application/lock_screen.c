#include "lock_screen.h"
#include "drv_lcd.h"
#include "drv_button.h"
#include "drv_touch.h"
#include "drv_power_switch.h"
#include "user_msg.h"
#include "lvgl.h"
#include "user_delay.h"
#include "user_utils.h"
#include "device_settings.h"

#define LOCK_SCREEN_TICK                                    1000
#define LOCK_SCREEN_TIME_MAX_SECONDS                        3600

static void ShortPressHandler(void);
static void ReleaseHandler(void);
static void LockScreenTimerFunc(void *argument);

static osTimerId_t g_lockScreenTimer;
static volatile uint32_t g_lockScreenTick;
static bool g_screenLocked;


void LockScreenInit(void)
{
    //Register short press and release button event.
    RegisterButtonEvent(BUTTON_EVENT_SHORT_PRESS, ShortPressHandler);
    RegisterButtonEvent(BUTTON_EVENT_RELEASE, ReleaseHandler);
    g_lockScreenTimer = osTimerNew(LockScreenTimerFunc, osTimerPeriodic, NULL, NULL);
    g_lockScreenTick = 0;
    g_screenLocked = false;
    osTimerStart(g_lockScreenTimer, LOCK_SCREEN_TICK);
}

void ClearLockScreenTime(void)
{
    g_lockScreenTick = 0;
}

void LockScreen(void)
{
    if (g_screenLocked) {
        return;
    }
    TouchOnOff(false);
    osTimerStop(g_lockScreenTimer);
    ClearLockScreenTime();
    SetLcdBackLight(0);
    //PowerSwitchSetSource(POWER_SOURCE_LCD, false);
    LcdClose();
    g_screenLocked = true;
}

void UnlockScreen(void)
{
    if (!g_screenLocked) {
        return;
    }
    osTimerStart(g_lockScreenTimer, LOCK_SCREEN_TICK);
    ClearLockScreenTime();
    TouchOnOff(true);
    //PowerSwitchSetSource(POWER_SOURCE_LCD, true);
    LcdOpen();
    PubValueMsg(UI_MSG_REFRESH, 0);
    SetLcdBackLight(DeviceSettingsGetBrightness());
    g_screenLocked = false;
}

static void ShortPressHandler(void)
{
}

static void ReleaseHandler(void)
{
    if (g_screenLocked) {
        PubValueMsg(BACKGROUND_MSG_UNLOCK_SCREEN, 0);
    } else {
        PubValueMsg(BACKGROUND_MSG_LOCK_SCREEN, 0);
    }
}

static void LockScreenTimerFunc(void *argument)
{
    uint32_t lockScreenTime;
    uint32_t lockScreenTimeout;

    UNUSED(argument);
    lockScreenTime = DeviceSettingsGetLockScreenTime();
    if (lockScreenTime > LOCK_SCREEN_TIME_MAX_SECONDS) {
        ClearLockScreenTime();
        return;
    }

    g_lockScreenTick += LOCK_SCREEN_TICK;
    lockScreenTimeout = lockScreenTime * LOCK_SCREEN_TICK;
    if (g_lockScreenTick >= lockScreenTimeout) {
        g_lockScreenTick = 0;
        PubValueMsg(BACKGROUND_MSG_LOCK_SCREEN, 0);
    }
}
