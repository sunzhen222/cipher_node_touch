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

typedef struct {
    lv_obj_t *imageTitle;
    lv_obj_t *buttonLoraChat;
    lv_obj_t *buttonMqttChat;
    lv_obj_t *buttonTouchTest;
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
    .fullScreen = false,
};


static void HomePageInit(void)
{
    HomePageValues_t *values = SRAM_MALLOC(sizeof(HomePageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    values->imageTitle = lv_image_create(GetPageBackground());
    lv_img_set_src(values->imageTitle, &cipher_node_touch_logo);
    lv_obj_align(values->imageTitle, LV_ALIGN_TOP_MID, 0, 24);

    values->buttonLoraChat = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonLoraChat, 100, 60);
    lv_obj_align(values->buttonLoraChat, LV_ALIGN_TOP_LEFT, 45, 140);
    lv_obj_t *btnLabel = lv_label_create(values->buttonLoraChat);
    lv_label_set_text(btnLabel, "LoRa Chat");
    lv_obj_add_event_cb(values->buttonLoraChat, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonMqttChat = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonMqttChat, 100, 60);
    lv_obj_align(values->buttonMqttChat, LV_ALIGN_TOP_RIGHT, -45, 140);
    lv_obj_t *mqttChatBtnLabel = lv_label_create(values->buttonMqttChat);
    lv_label_set_text(mqttChatBtnLabel, "MQTT Chat");
    lv_obj_add_event_cb(values->buttonMqttChat, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonTouchTest = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonTouchTest, 100, 60);
    lv_obj_align(values->buttonTouchTest, LV_ALIGN_TOP_LEFT, 45, 240);
    lv_obj_t *touchTestBtnLabel = lv_label_create(values->buttonTouchTest);
    lv_label_set_text(touchTestBtnLabel, "Touch Test");
    lv_obj_add_event_cb(values->buttonTouchTest, HomePageButtonEventHandler, LV_EVENT_CLICKED, NULL);

    values->buttonSystem = lv_button_create(GetPageBackground());
    lv_obj_set_size(values->buttonSystem, 100, 60);
    lv_obj_align(values->buttonSystem, LV_ALIGN_TOP_RIGHT, -45, 240);
    lv_obj_t *systemBtnLabel = lv_label_create(values->buttonSystem);
    lv_label_set_text(systemBtnLabel, "System");
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
        if (btn == values->buttonLoraChat) {
            printf("LoRa Chat button pressed\n");
            EnterNewPage(&g_loraChatPage);
        } else if (btn == values->buttonMqttChat) {
            printf("MQTT Chat button pressed\n");
            EnterNewPage(&g_mqttChatPage);
        } else if (btn == values->buttonTouchTest) {
            printf("Touch Test button pressed\n");
            EnterNewPage(&g_touchTestPage);
        } else if (btn == values->buttonSystem) {
            printf("System button pressed\n");
            EnterNewPage(&g_systemPage);
        }
    }
}
