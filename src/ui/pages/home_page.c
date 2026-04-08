#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "device_settings.h"
#include "images_declare.h"
#include "lv_i18n.h"

typedef struct {
    lv_obj_t *imageTitle;
    lv_obj_t *buttonConfig;
    lv_obj_t *buttonSpeedCtrl;
    lv_obj_t *buttonFirmware;
    lv_obj_t *buttonSystem;
} HomePageValues_t;


static void HomePageInit(void);
static void HomePageDeinit(void);
static void HomePageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void HomePageButtonEventHandler(lv_event_t *e);

Page_t g_homePage = {
    .init = HomePageInit,
    .deinit = HomePageDeinit,
    .msgHandler = HomePageMsgHandler,
    .fullScreen = true,
};


static void HomePageInit(void)
{
    HomePageValues_t *values = SRAM_MALLOC(sizeof(HomePageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    values->imageTitle = lv_image_create(GetPageBackground());
    lv_img_set_src(values->imageTitle, &cipher_node_touch_logo);
    lv_obj_align(values->imageTitle, LV_ALIGN_TOP_MID, 0, 15);

    values->buttonConfig = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonConfig, 100, 60);
    lv_obj_align(values->buttonConfig, LV_ALIGN_TOP_LEFT, 50, 80);
    lv_obj_t *btnLabel = lv_label_create(values->buttonConfig);
    lv_label_set_text(btnLabel, _("config"));
    lv_obj_add_event_cb(values->buttonConfig, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonSpeedCtrl = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonSpeedCtrl, 100, 60);
    lv_obj_align(values->buttonSpeedCtrl, LV_ALIGN_TOP_RIGHT, -50, 80);
    lv_obj_t *speedBtnLabel = lv_label_create(values->buttonSpeedCtrl);
    lv_label_set_text(speedBtnLabel, _("speed_ctrl"));
    lv_obj_add_event_cb(values->buttonSpeedCtrl, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonFirmware = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonFirmware, 100, 60);
    lv_obj_align(values->buttonFirmware, LV_ALIGN_TOP_LEFT, 50, 160);
    lv_obj_t *firmwareBtnLabel = lv_label_create(values->buttonFirmware);
    lv_label_set_text(firmwareBtnLabel, _("firmware"));
    lv_obj_add_event_cb(values->buttonFirmware, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonSystem = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonSystem, 100, 60);
    lv_obj_align(values->buttonSystem, LV_ALIGN_TOP_RIGHT, -50, 160);
    lv_obj_t *systemBtnLabel = lv_label_create(values->buttonSystem);
    lv_label_set_text(systemBtnLabel, _("system"));
    lv_obj_add_event_cb(values->buttonSystem, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);
}


static void HomePageDeinit(void)
{
    HomePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}


static void HomePageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}

static void HomePageButtonEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *btn = lv_event_get_target(e);
        HomePageValues_t *values = lv_obj_get_user_data(GetPageBackground());
        if (btn == values->buttonConfig) {
            printf("Config button pressed\n");
        } else if (btn == values->buttonSpeedCtrl) {
            printf("Speed Controller button pressed\n");
        } else if (btn == values->buttonFirmware) {
            printf("Firmware button pressed\n");
        } else if (btn == values->buttonSystem) {
            printf("System button pressed\n");
            EnterNewPage(&g_systemPage);
        }
    }
}
