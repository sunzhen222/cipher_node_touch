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
#include "background_task.h"
#include "confirm_win.h"
#include "loading_spinner.h"
#include "sha256.h"
#ifdef SIMULATOR
#include "SDL2/SDL.h"
#endif

#define STM32_UID_BASE             0x1FFF7A10
#define INTERNAL_FLASH_START       0x08000000U
#define FIRMWARE_FLASH_START       0x08020000U
#define INTERNAL_FLASH_END         0x08100000U
#define FLASH_END_FF_COUNT         128U
#define SHA256_RESULT_TEXT_SIZE    128U

typedef enum {
    FLASH_IMAGE_BOOTLOADER = 0,
    FLASH_IMAGE_FIRMWARE,
} FlashImage_t;

typedef struct {
    FlashImage_t image;
    uint8_t digest[SHA256_SIZE_BYTES];
    uint32_t length;
    bool success;
} FlashSha256Result_t;

typedef struct {
    lv_obj_t *menu;
    lv_obj_t *loadingSpinner;
    bool calculating;
} AboutPageValues_t;

static void AboutPageInit(void);
static void AboutPageDeinit(void);
static void AboutPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void BootloaderSha256Callback(lv_event_t *e);
static void FirmwareSha256Callback(lv_event_t *e);
static void StartFlashSha256(FlashImage_t image);
static int32_t CalculateFlashSha256(const void *inData, uint32_t inDataLen);

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
        {"Bootloader SHA256", BootloaderSha256Callback},
        {"Firmware SHA256", FirmwareSha256Callback},
    };
    values->menu = CreateUserMenu(menuItems, sizeof(menuItems) / sizeof(UserMenuItem_t));
}

static void AboutPageDeinit(void)
{
    AboutPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values) {
        if (values->loadingSpinner != NULL) {
            DeleteLoadingSpinner(values->loadingSpinner);
            values->loadingSpinner = NULL;
        }
        DestroyUserMenu(values->menu);
        SRAM_FREE(values);
    }
}

static void AboutPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    AboutPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    FlashSha256Result_t *result;
    ConfirmWin_t confirmWin = {0};
    char resultText[SHA256_RESULT_TEXT_SIZE];
    uint32_t offset;

    if (code != UI_MSG_CODE_FLASH_SHA256_RESULT || values == NULL ||
            data == NULL || dataLen != sizeof(FlashSha256Result_t)) {
        return;
    }

    if (values->loadingSpinner != NULL) {
        DeleteLoadingSpinner(values->loadingSpinner);
        values->loadingSpinner = NULL;
    }
    values->calculating = false;
    result = (FlashSha256Result_t *)data;
    if (!result->success) {
        confirmWin.text = "SHA256 is unavailable in simulator";
        CreateConfirmWin(GetPageBackground(), &confirmWin);
        return;
    }

    offset = (uint32_t)snprintf(resultText, sizeof(resultText), "%s length: %lu\nSHA256:\n",
                                result->image == FLASH_IMAGE_BOOTLOADER ? "Bootloader" : "Firmware",
                                (unsigned long)result->length);
    for (uint32_t i = 0; i < SHA256_SIZE_BYTES && offset + 2 < sizeof(resultText); i++) {
        offset += (uint32_t)snprintf(resultText + offset, sizeof(resultText) - offset, "%02x", result->digest[i]);
    }
    confirmWin.text = resultText;
    CreateConfirmWin(GetPageBackground(), &confirmWin);
}

static void BootloaderSha256Callback(lv_event_t *e)
{
    UNUSED(e);
    StartFlashSha256(FLASH_IMAGE_BOOTLOADER);
}

static void FirmwareSha256Callback(lv_event_t *e)
{
    UNUSED(e);
    StartFlashSha256(FLASH_IMAGE_FIRMWARE);
}

static void StartFlashSha256(FlashImage_t image)
{
    AboutPageValues_t *values = lv_obj_get_user_data(GetPageBackground());

    if (values == NULL || values->calculating) {
        return;
    }
    values->calculating = true;
    values->loadingSpinner = CreateLoadingSpinner(GetPageBackground(), 0);
    AsyncExecute(CalculateFlashSha256, &image, sizeof(image), 0);
}

static int32_t CalculateFlashSha256(const void *inData, uint32_t inDataLen)
{
    FlashSha256Result_t result = {0};

    if (inData == NULL || inDataLen != sizeof(FlashImage_t)) {
        return -1;
    }
    result.image = *(const FlashImage_t *)inData;
#ifdef SIMULATOR
    //delay 5 seconds to simulate the calculation time
    SDL_Delay(5000);
    result.success = false;
#else
    const uint8_t *current;
    const uint8_t *end;
    uint32_t pendingFf = 0;
    const uint8_t ff = 0xFF;
    sha256_context context;

    if (result.image == FLASH_IMAGE_BOOTLOADER) {
        current = (const uint8_t *)INTERNAL_FLASH_START;
        end = (const uint8_t *)FIRMWARE_FLASH_START;
    } else {
        current = (const uint8_t *)FIRMWARE_FLASH_START;
        end = (const uint8_t *)INTERNAL_FLASH_END;
    }

    sha256_init(&context);
    while (current < end) {
        if (*current == 0xFF) {
            pendingFf++;
            if (pendingFf >= FLASH_END_FF_COUNT) {
                break;
            }
        } else {
            while (pendingFf > 0) {
                sha256_hash(&context, &ff, 1);
                pendingFf--;
                result.length++;
            }
            sha256_hash(&context, current, 1);
            result.length++;
        }
        current++;
    }
    sha256_done(&context, result.digest);
    result.success = true;
#endif
    SendUiMsg(UI_MSG_CODE_FLASH_SHA256_RESULT, &result, sizeof(result));
    return 0;
}
