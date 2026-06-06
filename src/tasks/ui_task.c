#include "ui_task.h"
#include "cmsis_os.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "page.h"
#include "pages_declare.h"
#include "status_bar.h"
#include "drv_lcd.h"
#include "user_utils.h"
#include "user_msg.h"
#include "lv_theme_pocket.h"
#include "drv_timer.h"
#include "device_settings.h"
#include "snapshot.h"

#define LVGL_TICK                       5


osThreadId_t g_uiTaskHandle;
osTimerId_t g_lvglTickTimer;

static void UiTask(void *argument);
static void LvglTickTimerFunc(void *argument);


void CreateUiTask(void)
{
    const osThreadAttr_t testtTask_attributes = {
        .name = "UiTask",
        .stack_size = 1024 * 16,
        .priority = (osPriority_t) osPriorityHigh,
    };
    g_uiTaskHandle = osThreadNew(UiTask, NULL, &testtTask_attributes);
    g_lvglTickTimer = osTimerNew(LvglTickTimerFunc, osTimerPeriodic, NULL, NULL);
    osTimerStart(g_lvglTickTimer, LVGL_TICK);
}

static void UiTask(void *argument)
{
    UNUSED(argument);
    Message_t rcvMsg;
    osStatus_t ret;
    uint32_t startTick = 0;
    lv_display_t *disp;
    lv_theme_t *theme;

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    disp = lv_display_get_default();
    theme = lv_theme_pocket_init(disp);
    lv_display_set_theme(disp, theme);

    CreateStatusBar();
    EnterNewPage(&g_homePage);
    SetLcdBackLight(0);
    RegisterButtonSnapshot();

    while (1) {
        ret = osMessageQueueGet(g_uiQueue, &rcvMsg, NULL, 10);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case UI_MSG_USER_EVENT: {
                //printf("code=%lu,data_addr=%p,dataLen=%lu\n", rcvMsg.value, rcvMsg.buffer, rcvMsg.length);
                HandleCurrentPageMsg(rcvMsg.value, rcvMsg.buffer, rcvMsg.length);
                HandleStatusBarMsg(rcvMsg.value, rcvMsg.buffer, rcvMsg.length);
            }
            break;
            case UI_MSG_HOME: {
                BackToRootPage();
            }
            break;
            case UI_MSG_REFRESH: {
                lv_obj_invalidate(lv_screen_active());
                lv_refr_now(disp);
            }
            break;
            case UI_MSG_RELOAD_THEME: {
                printf("reload UI theme\n");
                theme = lv_theme_pocket_init(disp);
                lv_display_set_theme(disp, theme);
            }
            break;
            default:
                break;
            }
            if (rcvMsg.buffer != NULL) {
                SRAM_FREE(rcvMsg.buffer);
            }
        }
        lv_timer_handler();
        if (startTick < 10) {
            startTick++;
            if (startTick == 10) {
                SetLcdBackLight(DeviceSettingsGetBrightness());
            }
        }
    }
}


static void LvglTickTimerFunc(void *argument)
{
    UNUSED(argument);
    lv_tick_inc(LVGL_TICK);
}

