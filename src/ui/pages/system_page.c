#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "user_menu.h"
#include "confirm_win.h"
#include "stm32f4xx_hal.h"
#include "drv_w25qxx.h"

static void SystemPageInit(void);
static void SystemPageDeinit(void);
static void SystemPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

typedef struct {
    lv_obj_t *menu;
    uint32_t menuItemCount;
} SystemPageValues_t;

static void WidgetColorCallback(lv_event_t *e);
static void LcdBrightnessCallback(lv_event_t *e);
static void WifiPageCallback(lv_event_t *e);
static void FactoryResetCallback(lv_event_t *e);
static void AboutCallback(lv_event_t *e);
static void FactoryResetOkHandler(lv_event_t *e);

Page_t g_systemPage = {
    .init = SystemPageInit,
    .deinit = SystemPageDeinit,
    .msgHandler = SystemPageMsgHandler,
    .fullScreen = false,
};

static void SystemPageInit(void)
{
    SystemPageValues_t *values = SRAM_MALLOC(sizeof(SystemPageValues_t));
    memset(values, 0, sizeof(SystemPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    UserMenuItem_t menuItems[] = {
        {"Widget color", WidgetColorCallback},
        {"LCD brightness", LcdBrightnessCallback},
        {"WiFi", WifiPageCallback},
        {"Factory Reset", FactoryResetCallback},
        {"About", AboutCallback},
    };
    values->menuItemCount = sizeof(menuItems) / sizeof(UserMenuItem_t);
    values->menu = CreateUserMenu(menuItems, values->menuItemCount);
}

static void SystemPageDeinit(void)
{
    SystemPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values) {
        DestroyUserMenu(values->menu);
        SRAM_FREE(values);
    }
}

static void SystemPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void WidgetColorCallback(lv_event_t *e)
{
    UNUSED(e);
}

static void LcdBrightnessCallback(lv_event_t *e)
{
    UNUSED(e);
    EnterNewPage(&g_lcdBrightnessPage);
}

static void WifiPageCallback(lv_event_t *e)
{
    UNUSED(e);
    EnterNewPage(&g_wifiPage);
}

static void FactoryResetCallback(lv_event_t *e)
{
    UNUSED(e);
    ConfirmWin_t confirmWin = {0};
    confirmWin.text = "Are you sure to reset to factory settings?";
    confirmWin.OkHandler = FactoryResetOkHandler;
    CreateConfirmWin(GetPageBackground(), &confirmWin);
}

static void AboutCallback(lv_event_t *e)
{
    UNUSED(e);
    EnterNewPage(&g_aboutPage);
}

static void FactoryResetOkHandler(lv_event_t *e)
{
    UNUSED(e);
    // Erase W25Qxx flash chip
    W25qxx_EraseChip();
    NVIC_SystemReset();
}
