#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"

typedef struct {
    lv_obj_t *frequencyInput;
} ChatSettingsPageValues_t;


static void ChatSettingsPageInit(void);
static void ChatSettingsPageDeinit(void);
static void ChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);

Page_t g_chatSettingsPage = {
    .init = ChatSettingsPageInit,
    .deinit = ChatSettingsPageDeinit,
    .msgHandler = ChatSettingsPageMsgHandler,
    .fullScreen = false,
};


static void ChatSettingsPageInit(void)
{
    CreateGeneralNavigationBar();
    ChatSettingsPageValues_t *values = SRAM_MALLOC(sizeof(ChatSettingsPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    values->frequencyInput = lv_textarea_create(GetPageBackground());
    lv_textarea_set_placeholder_text(values->frequencyInput, "433000000");
    lv_obj_set_size(values->frequencyInput, 200, 40);
    lv_obj_align(values->frequencyInput, LV_ALIGN_TOP_MID, 0, 30);
}


static void ChatSettingsPageDeinit(void)
{
    ChatSettingsPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    SRAM_FREE(values);
}


static void ChatSettingsPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    UNUSED(data);
    UNUSED(dataLen);
    UNUSED(code);
    //switch (code) {
    //default:
    //    break;
    //}
}
