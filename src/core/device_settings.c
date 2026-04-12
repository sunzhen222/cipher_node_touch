#include "device_settings.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "drv_w25qxx.h"
#include "assert.h"
#include "user_memory.h"
#include "cJSON.h"
#include "user_utils.h"
#include "test_cmd.h"
#include "flash_map.h"
#include "lvgl.h"
#include "lora.h"


#define VERSION_MAX_LENGTH                  32

#define KEY_VERSION                         "version"

#define KEY_BRIGHTNESS                      "brightness"
#define KEY_SPEED_CTRL_MODE                 "speed_ctrl_mode"
#define KEY_SPEED_CTRL_SMOOTH               "speed_ctrl_smooth"
#define KEY_WIDGET_COLOR                    "widget_color"
#define KEY_LANGUAGE                        "language"
#define KEY_LORA_FREQ                       "lora_freq"
#define KEY_DEVICE_ID                       "device_id"
#define KEY_LORA_NET_ID                     "lora_net_id"
#define KEY_LORA_SECRET_KEY                 "lora_secret_key"
#define KEY_LORA_SPREADING_FACTOR           "lora_sf"
#define KEY_LORA_BANDWIDTH                  "lora_bw"

#define DEFAULT_BRIGHTNESS                  100
#define DEFAULT_SPEED_CTRL_MODE             0
#define DEFAULT_SPEED_CTRL_SMOOTH           1
#define DEFAULT_WIDGET_COLOR                LV_PALETTE_BLUE
#define DEFAULT_LANGUAGE                    0
#define DEFAULT_LORA_FREQ                   434000000
#define DEFAULT_DEVICE_ID                   1
#define DEFAULT_LORA_NET_ID                 0
#define DEFAULT_LORA_SECRET_KEY             "default_lora_secret_key"
#define DEFAULT_LORA_SPREADING_FACTOR       LLCC68_LORA_DEFAULT_SF
#define DEFAULT_LORA_BANDWIDTH              LLCC68_LORA_DEFAULT_BANDWIDTH

typedef struct {
    uint32_t brightness;
    uint32_t speedCtrlMode;
    uint32_t speedCtrlSmooth;
    uint32_t widgetColor;
    uint32_t language;
    uint32_t loraFreq;
    uint32_t deviceId;
    uint32_t loraNetId;
    char loraSecretKey[96];
    uint32_t loraSpreadingFactor;
    uint32_t loraBandwidth;
} DeviceSettings_t;

static void SaveDeviceSettingsSync(void);
static bool GetDeviceSettingsFromJsonString(const char *string);
static char *GetJsonStringFromDeviceSettings(void);

static const char g_deviceSettingsVersion[] = "0.0.1";
static DeviceSettings_t g_deviceSettings;

void DeviceSettingsInit(void)
{
    uint32_t size;
    bool needRegenerate = false;
    char *jsonString = NULL;

    W25qxx_ReadBytes((uint8_t *)&size, FLASH_ADDR_DEVICE_SETTINGS, sizeof(size));
    printf("device settings size=%lu\n", size);
    do {
        if (size >= FLASH_SIZE_DEVICE_SETTINGS - 4) {
            needRegenerate = true;
            printf("device settings size err,%lu\n", size);
            break;
        }
        jsonString = SRAM_MALLOC(size + 1);
        W25qxx_ReadBytes((uint8_t *)jsonString, FLASH_ADDR_DEVICE_SETTINGS + 4, size);
        jsonString[size] = 0;
        if (GetDeviceSettingsFromJsonString(jsonString) == false) {
            printf("GetDeviceSettingsFromJsonString false, need regenerate\n");
            printf("err jsonString=%s\n", jsonString);
            needRegenerate = true;
        }
        SRAM_FREE(jsonString);
    } while (0);

    if (needRegenerate) {
        printf("regenerate settings\n");
        g_deviceSettings.brightness = DEFAULT_BRIGHTNESS;
        g_deviceSettings.speedCtrlMode = DEFAULT_SPEED_CTRL_MODE;
        g_deviceSettings.speedCtrlSmooth = DEFAULT_SPEED_CTRL_SMOOTH;
        g_deviceSettings.widgetColor = DEFAULT_WIDGET_COLOR;
        g_deviceSettings.language = DEFAULT_LANGUAGE;
        g_deviceSettings.loraFreq = DEFAULT_LORA_FREQ;
        g_deviceSettings.deviceId = DEFAULT_DEVICE_ID;
        g_deviceSettings.loraNetId = DEFAULT_LORA_NET_ID;
        strncpy(g_deviceSettings.loraSecretKey, DEFAULT_LORA_SECRET_KEY, sizeof(g_deviceSettings.loraSecretKey) - 1);
        g_deviceSettings.loraSecretKey[sizeof(g_deviceSettings.loraSecretKey) - 1] = '\0';
        g_deviceSettings.loraSpreadingFactor = DEFAULT_LORA_SPREADING_FACTOR;
        g_deviceSettings.loraBandwidth = DEFAULT_LORA_BANDWIDTH;
        SaveDeviceSettingsSync();
    }
    PrintDeviceSettings();
    RegisterTestCmd("device_settings:", DeviceSettingsTest);
}

uint32_t DeviceSettingsGetBrightness(void)
{
    return g_deviceSettings.brightness;
}

void DeviceSettingsSetBrightness(uint32_t brightness)
{
    g_deviceSettings.brightness = brightness;
}

uint32_t DeviceSettingsGetSpeedCtrlMode(void)
{
    return g_deviceSettings.speedCtrlMode;
}

void DeviceSettingsSetSpeedCtrlMode(uint32_t mode)
{
    g_deviceSettings.speedCtrlMode = mode;
}

uint32_t DeviceSettingsGetSpeedCtrlSmooth(void)
{
    return g_deviceSettings.speedCtrlSmooth;
}

void DeviceSettingsSetSpeedCtrlSmooth(uint32_t smooth)
{
    g_deviceSettings.speedCtrlSmooth = smooth;
}

uint32_t DeviceSettingsGetWidgetColor(void)
{
    return g_deviceSettings.widgetColor;
}

void DeviceSettingsSetWidgetColor(uint32_t color)
{
    g_deviceSettings.widgetColor = color;
}

uint32_t DeviceSettingsGetLanguage(void)
{
    return g_deviceSettings.language;
}

void DeviceSettingsSetLanguage(uint32_t language)
{
    g_deviceSettings.language = language;
}

uint32_t DeviceSettingsGetLoraFreq(void)
{
    return g_deviceSettings.loraFreq;
}

void DeviceSettingsSetLoraFreq(uint32_t freq)
{
    g_deviceSettings.loraFreq = freq;
}

uint32_t DeviceSettingsGetDeviceId(void)
{
    return g_deviceSettings.deviceId;
}

void DeviceSettingsSetDeviceId(uint32_t id)
{
    g_deviceSettings.deviceId = id;
}

uint32_t DeviceSettingsGetLoraNetId(void)
{
    return g_deviceSettings.loraNetId;
}

void DeviceSettingsSetLoraNetId(uint32_t id)
{
    g_deviceSettings.loraNetId = id;
}

const char *DeviceSettingsGetLoraSecretKey(void)
{
    return g_deviceSettings.loraSecretKey;
}

void DeviceSettingsSetLoraSecretKey(const char *secretKey)
{
    if (secretKey == NULL) {
        return;
    }
    strncpy(g_deviceSettings.loraSecretKey, secretKey, sizeof(g_deviceSettings.loraSecretKey) - 1);
    g_deviceSettings.loraSecretKey[sizeof(g_deviceSettings.loraSecretKey) - 1] = '\0';
}

uint32_t DeviceSettingsGetLoraSpreadingFactor(void)
{
    return g_deviceSettings.loraSpreadingFactor;
}

void DeviceSettingsSetLoraSpreadingFactor(uint32_t sf)
{
    g_deviceSettings.loraSpreadingFactor = sf;
}

uint32_t DeviceSettingsGetLoraBandwidth(void)
{
    return g_deviceSettings.loraBandwidth;
}

void DeviceSettingsSetLoraBandwidth(uint32_t bw)
{
    g_deviceSettings.loraBandwidth = bw;
}

void SaveDeviceSettings(void)
{
    SaveDeviceSettingsSync();
}

void PrintDeviceSettings(void)
{
}

void DeviceSettingsTest(int argc, char *argv[])
{
    if (argc == 0) {
        printf("device settings test err\n");
        return;
    }
    if (strcmp(argv[0], "save") == 0) {
        SaveDeviceSettings();
    } else if (strcmp(argv[0], "print") == 0) {
        PrintDeviceSettings();
    } else if (strcmp(argv[0], "json") == 0) {
        char *jsonString;
        uint32_t size;
        W25qxx_ReadBytes((uint8_t *)&size, FLASH_ADDR_DEVICE_SETTINGS, sizeof(size));
        if (size >= FLASH_SIZE_DEVICE_SETTINGS - 4) {
            printf("device settings size err,%lu\n", size);
            return;
        }
        jsonString = SRAM_MALLOC(size + 1);
        W25qxx_ReadBytes((uint8_t *)jsonString, FLASH_ADDR_DEVICE_SETTINGS + 4, size);
        jsonString[size] = 0;
        printf("jsonString=%s\n", jsonString);
        SRAM_FREE(jsonString);
    }
}

static void SaveDeviceSettingsSync(void)
{
    char *jsonString;
    uint32_t size;

    jsonString = GetJsonStringFromDeviceSettings();
    printf("jsonString=%s\n", jsonString);
    W25qxx_EraseAddr(FLASH_ADDR_DEVICE_SETTINGS);      //Only one sector for device settings.
    size = strlen(jsonString);
    ASSERT(size < 4091);
    W25qxx_WriteBytes((uint8_t *)&size, FLASH_ADDR_DEVICE_SETTINGS, 4);
    W25qxx_WriteBytes((uint8_t *)jsonString, FLASH_ADDR_DEVICE_SETTINGS + 4, size + 1);
    SRAM_FREE(jsonString);
}

static bool GetDeviceSettingsFromJsonString(const char *string)
{
    cJSON *rootJson;
    bool ret = true;
    char versionString[VERSION_MAX_LENGTH];

    do {
        rootJson = cJSON_Parse(string);
        if (rootJson == NULL) {
            printf("device settings json parse fail\n");
            ret = false;
            break;
        }
        GetStringValue(rootJson, KEY_VERSION, "", versionString);
        if (strcmp(versionString, g_deviceSettingsVersion) != 0) {
            printf("saved version:%s\n", versionString);
            printf("g_deviceSettingsVersion:%s\n", g_deviceSettingsVersion);
            ret = false;
            break;
        }
        g_deviceSettings.brightness = GetIntValue(rootJson, KEY_BRIGHTNESS, DEFAULT_BRIGHTNESS);
        g_deviceSettings.speedCtrlMode = GetIntValue(rootJson, KEY_SPEED_CTRL_MODE, DEFAULT_SPEED_CTRL_MODE);
        g_deviceSettings.speedCtrlSmooth = GetIntValue(rootJson, KEY_SPEED_CTRL_SMOOTH, DEFAULT_SPEED_CTRL_SMOOTH);
        g_deviceSettings.widgetColor = GetIntValue(rootJson, KEY_WIDGET_COLOR, DEFAULT_WIDGET_COLOR);
        g_deviceSettings.language = GetIntValue(rootJson, KEY_LANGUAGE, DEFAULT_LANGUAGE);
        g_deviceSettings.loraFreq = GetIntValue(rootJson, KEY_LORA_FREQ, DEFAULT_LORA_FREQ);
        g_deviceSettings.deviceId = GetIntValue(rootJson, KEY_DEVICE_ID, DEFAULT_DEVICE_ID);
        g_deviceSettings.loraNetId = GetIntValue(rootJson, KEY_LORA_NET_ID, DEFAULT_LORA_NET_ID);
        GetStringValue(rootJson, KEY_LORA_SECRET_KEY, DEFAULT_LORA_SECRET_KEY, g_deviceSettings.loraSecretKey);
        g_deviceSettings.loraSpreadingFactor = GetIntValue(rootJson, KEY_LORA_SPREADING_FACTOR, DEFAULT_LORA_SPREADING_FACTOR);
        g_deviceSettings.loraBandwidth = GetIntValue(rootJson, KEY_LORA_BANDWIDTH, DEFAULT_LORA_BANDWIDTH);
    } while (0);
    cJSON_Delete(rootJson);

    return ret;
}


static char *GetJsonStringFromDeviceSettings(void)
{
    cJSON *rootJson;
    char *retStr;

    rootJson = cJSON_CreateObject();
    cJSON_AddItemToObject(rootJson, KEY_VERSION, cJSON_CreateString(g_deviceSettingsVersion));
    cJSON_AddItemToObject(rootJson, KEY_BRIGHTNESS, cJSON_CreateNumber(g_deviceSettings.brightness));
    cJSON_AddItemToObject(rootJson, KEY_SPEED_CTRL_MODE, cJSON_CreateNumber(g_deviceSettings.speedCtrlMode));
    cJSON_AddItemToObject(rootJson, KEY_SPEED_CTRL_SMOOTH, cJSON_CreateNumber(g_deviceSettings.speedCtrlSmooth));
    cJSON_AddItemToObject(rootJson, KEY_WIDGET_COLOR, cJSON_CreateNumber(g_deviceSettings.widgetColor));
    cJSON_AddItemToObject(rootJson, KEY_LANGUAGE, cJSON_CreateNumber(g_deviceSettings.language));
    cJSON_AddItemToObject(rootJson, KEY_LORA_FREQ, cJSON_CreateNumber(g_deviceSettings.loraFreq));
    cJSON_AddItemToObject(rootJson, KEY_DEVICE_ID, cJSON_CreateNumber(g_deviceSettings.deviceId));
    cJSON_AddItemToObject(rootJson, KEY_LORA_NET_ID, cJSON_CreateNumber(g_deviceSettings.loraNetId));
    cJSON_AddItemToObject(rootJson, KEY_LORA_SECRET_KEY, cJSON_CreateString(g_deviceSettings.loraSecretKey));
    cJSON_AddItemToObject(rootJson, KEY_LORA_SPREADING_FACTOR, cJSON_CreateNumber(g_deviceSettings.loraSpreadingFactor));
    cJSON_AddItemToObject(rootJson, KEY_LORA_BANDWIDTH, cJSON_CreateNumber(g_deviceSettings.loraBandwidth));
    retStr = cJSON_Print(rootJson);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);

    return retStr;
}
