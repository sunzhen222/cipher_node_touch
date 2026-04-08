#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "user_menu.h"
#include "lv_i18n.h"
#include "device_settings.h"

typedef struct {
    lv_obj_t *menu;
} LanguagePageValues_t;


static void LanguagePageInit(void);
static void LanguagePageDeinit(void);
static void LanguagePageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void EnglishCallback(lv_event_t *e);
static void ChineseCallback(lv_event_t *e);
static void KoreanCallback(lv_event_t *e);

Page_t g_languagePage = {
    .init = LanguagePageInit,
    .deinit = LanguagePageDeinit,
    .msgHandler = LanguagePageMsgHandler,
    .fullScreen = true,
};


static void LanguagePageInit(void)
{
    LanguagePageValues_t *values = SRAM_MALLOC(sizeof(LanguagePageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    UserMenuItem_t items[] = {
        {_("language_english"), EnglishCallback},
        {_("language_chinese"), ChineseCallback},
        {_("language_korean"), KoreanCallback},
    };

    values->menu = CreateUserMenu(items, sizeof(items) / sizeof(UserMenuItem_t));
    SetUserMenuItemChecked(values->menu, DeviceSettingsGetLanguage(), true);
}

static void LanguagePageDeinit(void)
{
    LanguagePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    DestroyUserMenu(values->menu);
    SRAM_FREE(values);
}

static void LanguagePageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}


static void EnglishCallback(lv_event_t *e)
{
    UNUSED(e);
    LanguagePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SetUserMenuItemChecked(values->menu, 0, true);
    SetUserMenuItemChecked(values->menu, 1, false);
    SetUserMenuItemChecked(values->menu, 2, false);
    DeviceSettingsSetLanguage(0);
    SaveDeviceSettings();
    lv_i18n_set_locale("en");
    printf("en selected\n");
}


static void ChineseCallback(lv_event_t *e)
{
    UNUSED(e);
    LanguagePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SetUserMenuItemChecked(values->menu, 0, false);
    SetUserMenuItemChecked(values->menu, 1, true);
    SetUserMenuItemChecked(values->menu, 2, false);
    DeviceSettingsSetLanguage(1);
    SaveDeviceSettings();
    lv_i18n_set_locale("zh-cn");
    printf("zh-cn selected\n");
}


static void KoreanCallback(lv_event_t *e)
{
    UNUSED(e);
    LanguagePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SetUserMenuItemChecked(values->menu, 0, false);
    SetUserMenuItemChecked(values->menu, 1, false);
    SetUserMenuItemChecked(values->menu, 2, true);
    DeviceSettingsSetLanguage(2);
    SaveDeviceSettings();
    lv_i18n_set_locale("ko");
    printf("ko selected\n");
}
