#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "user_menu.h"
#include "hardware_version.h"
#include "software_version.h"
#include "drv_w25qxx.h"

#define STM32_UID_BASE  0x1FFF7A10

typedef struct {
    lv_obj_t *menu;
} AboutPageValues_t;

static void AboutPageInit(void);
static void AboutPageDeinit(void);
static void AboutPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

Page_t g_aboutPage = {
    .init = AboutPageInit,
    .deinit = AboutPageDeinit,
    .msgHandler = AboutPageMsgHandler,
    .fullScreen = false,
};

static void AboutPageInit(void)
{
    AboutPageValues_t *values = SRAM_MALLOC(sizeof(AboutPageValues_t));
    memset(values, 0, sizeof(AboutPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    CreateGeneralNavigationBar();

    char hardwareVersion[64];
    char softwareVersion[64];
    char buildTime[64];
    char uidStr[128];

    snprintf(hardwareVersion, sizeof(hardwareVersion), "Hardware version: %s", GetHardwareVersionString());
    snprintf(softwareVersion, sizeof(softwareVersion), "Software version: %s", GetSoftwareVersionString());
    snprintf(buildTime, sizeof(buildTime), "Build time: %s", GetBuildTime());

    uint32_t uid[3];
#ifdef SIMULATOR
    uid[0] = 0x53494D55;
    uid[1] = 0x4C41544FU;
    uid[2] = 0x52303031U;
#else
    uid[0] = *(uint32_t *)(STM32_UID_BASE + 0);
    uid[1] = *(uint32_t *)(STM32_UID_BASE + 4);
    uid[2] = *(uint32_t *)(STM32_UID_BASE + 8);
#endif
    snprintf(uidStr, sizeof(uidStr), "UID: %08lX %08lX %08lX", uid[0], uid[1], uid[2]);

    UserMenuItem_t menuItems[] = {
        {hardwareVersion, NULL},
        {softwareVersion, NULL},
        {buildTime, NULL},
        {uidStr, NULL},
    };
    values->menu = CreateUserMenu(menuItems, sizeof(menuItems) / sizeof(UserMenuItem_t));
}

static void AboutPageDeinit(void)
{
    AboutPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values) {
        DestroyUserMenu(values->menu);
        SRAM_FREE(values);
    }
}

static void AboutPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}
