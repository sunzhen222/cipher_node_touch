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
#include "lora.h"


#define VERSION_MAX_LENGTH                  32

#define KEY_VERSION                         "version"

#define KEY_BRIGHTNESS                      "brightness"
#define KEY_LORA_CHAT_AVATAR_COLOR          "lora_chat_avatar_color"
#define KEY_LORA_CHAT_USERNAME              "lora_chat_username"
#define KEY_MQTT_CHAT_AVATAR_COLOR          "mqtt_chat_avatar_color"
#define KEY_MQTT_CHAT_USERNAME              "mqtt_chat_username"
#define KEY_LORA_FREQ                       "lora_freq"
#define KEY_DEVICE_ID                       "device_id"
#define KEY_LORA_NET_ID                     "lora_net_id"
#define KEY_LORA_SECRET_KEY                 "lora_secret_key"
#define KEY_LORA_SPREADING_FACTOR           "lora_sf"
#define KEY_LORA_BANDWIDTH                  "lora_bw"
#define KEY_MQTT_BROKER_HOST                "mqtt_broker_host"
#define KEY_MQTT_BROKER_PORT                "mqtt_broker_port"
#define KEY_MQTT_TLS_MODE                   "mqtt_tls_mode"
#define KEY_MQTT_SUBSCRIBE_TOPIC            "mqtt_subscribe_topic"
#define KEY_MQTT_SUBSCRIBE_QOS              "mqtt_subscribe_qos"
#define KEY_MQTT_PUBLISH_TIMEOUT_MS         "mqtt_publish_timeout_ms"

#define DEFAULT_BRIGHTNESS                  100
#define DEFAULT_LORA_CHAT_AVATAR_COLOR      0x2FB35A
#define DEFAULT_LORA_CHAT_USERNAME          "CipherMan"
#define DEFAULT_MQTT_CHAT_AVATAR_COLOR      0x2FB35A
#define DEFAULT_MQTT_CHAT_USERNAME          "CipherMan"
#define DEFAULT_LORA_FREQ                   434000000
#define DEFAULT_DEVICE_ID                   1
#define DEFAULT_LORA_NET_ID                 0
#define DEFAULT_LORA_SECRET_KEY             "default_lora_secret_key"
#define DEFAULT_LORA_SPREADING_FACTOR       LLCC68_LORA_DEFAULT_SF
#define DEFAULT_LORA_BANDWIDTH              LLCC68_LORA_DEFAULT_BANDWIDTH
#define DEFAULT_MQTT_BROKER_HOST            "t1bf11cf.ala.cn-shenzhen.emqxsl.cn"
#define DEFAULT_MQTT_BROKER_PORT            8883
#define DEFAULT_MQTT_TLS_MODE               2
#define DEFAULT_MQTT_SUBSCRIBE_TOPIC        "testtopic/chat"
#define DEFAULT_MQTT_SUBSCRIBE_QOS          0
#define DEFAULT_MQTT_PUBLISH_TIMEOUT_MS     5000

typedef struct {
    uint32_t brightness;
    uint32_t loraChatAvatarColor;
    char loraChatUsername[32];
    uint32_t mqttChatAvatarColor;
    char mqttChatUsername[32];
    uint32_t loraFreq;
    uint32_t deviceId;
    uint32_t loraNetId;
    char loraSecretKey[96];
    uint32_t loraSpreadingFactor;
    uint32_t loraBandwidth;
    char mqttBrokerHost[64];
    uint32_t mqttBrokerPort;
    uint32_t mqttTlsMode;
    char mqttSubscribeTopic[64];
    uint32_t mqttSubscribeQos;
    uint32_t mqttPublishTimeoutMs;
} DeviceSettings_t;

static void SaveDeviceSettingsSync(void);
static bool GetDeviceSettingsFromJsonString(const char *string);
static char *GetJsonStringFromDeviceSettings(void);
static void SetDefaultDeviceSettings(void);
static void CopyStringValue(char *dst, size_t dstSize, const char *src);

static const char g_deviceSettingsVersion[] = "0.0.4";
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
        SetDefaultDeviceSettings();
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

uint32_t DeviceSettingsGetLoraChatAvatarColor(void)
{
    return g_deviceSettings.loraChatAvatarColor;
}

void DeviceSettingsSetLoraChatAvatarColor(uint32_t color)
{
    g_deviceSettings.loraChatAvatarColor = color;
}

const char *DeviceSettingsGetLoraChatUsername(void)
{
    return g_deviceSettings.loraChatUsername;
}

void DeviceSettingsSetLoraChatUsername(const char *username)
{
    if (username == NULL) {
        return;
    }

    strncpy(g_deviceSettings.loraChatUsername, username, sizeof(g_deviceSettings.loraChatUsername) - 1);
    g_deviceSettings.loraChatUsername[sizeof(g_deviceSettings.loraChatUsername) - 1] = '\0';
}

uint32_t DeviceSettingsGetMqttChatAvatarColor(void)
{
    return g_deviceSettings.mqttChatAvatarColor;
}

void DeviceSettingsSetMqttChatAvatarColor(uint32_t color)
{
    g_deviceSettings.mqttChatAvatarColor = color;
}

const char *DeviceSettingsGetMqttChatUsername(void)
{
    return g_deviceSettings.mqttChatUsername;
}

void DeviceSettingsSetMqttChatUsername(const char *username)
{
    CopyStringValue(g_deviceSettings.mqttChatUsername, sizeof(g_deviceSettings.mqttChatUsername), username);
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

const char *DeviceSettingsGetMqttBrokerHost(void)
{
    return g_deviceSettings.mqttBrokerHost;
}

void DeviceSettingsSetMqttBrokerHost(const char *host)
{
    CopyStringValue(g_deviceSettings.mqttBrokerHost, sizeof(g_deviceSettings.mqttBrokerHost), host);
}

uint32_t DeviceSettingsGetMqttBrokerPort(void)
{
    return g_deviceSettings.mqttBrokerPort;
}

void DeviceSettingsSetMqttBrokerPort(uint32_t port)
{
    g_deviceSettings.mqttBrokerPort = port;
}

uint32_t DeviceSettingsGetMqttTlsMode(void)
{
    return g_deviceSettings.mqttTlsMode;
}

void DeviceSettingsSetMqttTlsMode(uint32_t tlsMode)
{
    g_deviceSettings.mqttTlsMode = tlsMode;
}

const char *DeviceSettingsGetMqttSubscribeTopic(void)
{
    return g_deviceSettings.mqttSubscribeTopic;
}

void DeviceSettingsSetMqttSubscribeTopic(const char *topic)
{
    CopyStringValue(g_deviceSettings.mqttSubscribeTopic, sizeof(g_deviceSettings.mqttSubscribeTopic), topic);
}

uint32_t DeviceSettingsGetMqttSubscribeQos(void)
{
    return g_deviceSettings.mqttSubscribeQos;
}

void DeviceSettingsSetMqttSubscribeQos(uint32_t qos)
{
    g_deviceSettings.mqttSubscribeQos = qos;
}

uint32_t DeviceSettingsGetMqttPublishTimeoutMs(void)
{
    return g_deviceSettings.mqttPublishTimeoutMs;
}

void DeviceSettingsSetMqttPublishTimeoutMs(uint32_t timeoutMs)
{
    g_deviceSettings.mqttPublishTimeoutMs = timeoutMs;
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
        SetDefaultDeviceSettings();
        g_deviceSettings.brightness = GetIntValue(rootJson, KEY_BRIGHTNESS, DEFAULT_BRIGHTNESS);
        g_deviceSettings.loraChatAvatarColor = GetIntValue(rootJson, KEY_LORA_CHAT_AVATAR_COLOR, DEFAULT_LORA_CHAT_AVATAR_COLOR);
        GetStringValue(rootJson, KEY_LORA_CHAT_USERNAME, DEFAULT_LORA_CHAT_USERNAME, g_deviceSettings.loraChatUsername);
        g_deviceSettings.mqttChatAvatarColor = GetIntValue(rootJson, KEY_MQTT_CHAT_AVATAR_COLOR, DEFAULT_MQTT_CHAT_AVATAR_COLOR);
        GetStringValue(rootJson, KEY_MQTT_CHAT_USERNAME, DEFAULT_MQTT_CHAT_USERNAME, g_deviceSettings.mqttChatUsername);
        g_deviceSettings.loraFreq = GetIntValue(rootJson, KEY_LORA_FREQ, DEFAULT_LORA_FREQ);
        g_deviceSettings.deviceId = GetIntValue(rootJson, KEY_DEVICE_ID, DEFAULT_DEVICE_ID);
        g_deviceSettings.loraNetId = GetIntValue(rootJson, KEY_LORA_NET_ID, DEFAULT_LORA_NET_ID);
        GetStringValue(rootJson, KEY_LORA_SECRET_KEY, DEFAULT_LORA_SECRET_KEY, g_deviceSettings.loraSecretKey);
        g_deviceSettings.loraSpreadingFactor = GetIntValue(rootJson, KEY_LORA_SPREADING_FACTOR, DEFAULT_LORA_SPREADING_FACTOR);
        g_deviceSettings.loraBandwidth = GetIntValue(rootJson, KEY_LORA_BANDWIDTH, DEFAULT_LORA_BANDWIDTH);
        GetStringValue(rootJson, KEY_MQTT_BROKER_HOST, DEFAULT_MQTT_BROKER_HOST, g_deviceSettings.mqttBrokerHost);
        g_deviceSettings.mqttBrokerPort = GetIntValue(rootJson, KEY_MQTT_BROKER_PORT, DEFAULT_MQTT_BROKER_PORT);
        g_deviceSettings.mqttTlsMode = GetIntValue(rootJson, KEY_MQTT_TLS_MODE, DEFAULT_MQTT_TLS_MODE);
        GetStringValue(rootJson, KEY_MQTT_SUBSCRIBE_TOPIC, DEFAULT_MQTT_SUBSCRIBE_TOPIC, g_deviceSettings.mqttSubscribeTopic);
        g_deviceSettings.mqttSubscribeQos = GetIntValue(rootJson, KEY_MQTT_SUBSCRIBE_QOS, DEFAULT_MQTT_SUBSCRIBE_QOS);
        g_deviceSettings.mqttPublishTimeoutMs = GetIntValue(rootJson, KEY_MQTT_PUBLISH_TIMEOUT_MS, DEFAULT_MQTT_PUBLISH_TIMEOUT_MS);
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
    cJSON_AddItemToObject(rootJson, KEY_LORA_CHAT_AVATAR_COLOR, cJSON_CreateNumber(g_deviceSettings.loraChatAvatarColor));
    cJSON_AddItemToObject(rootJson, KEY_LORA_CHAT_USERNAME, cJSON_CreateString(g_deviceSettings.loraChatUsername));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_CHAT_AVATAR_COLOR, cJSON_CreateNumber(g_deviceSettings.mqttChatAvatarColor));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_CHAT_USERNAME, cJSON_CreateString(g_deviceSettings.mqttChatUsername));
    cJSON_AddItemToObject(rootJson, KEY_LORA_FREQ, cJSON_CreateNumber(g_deviceSettings.loraFreq));
    cJSON_AddItemToObject(rootJson, KEY_DEVICE_ID, cJSON_CreateNumber(g_deviceSettings.deviceId));
    cJSON_AddItemToObject(rootJson, KEY_LORA_NET_ID, cJSON_CreateNumber(g_deviceSettings.loraNetId));
    cJSON_AddItemToObject(rootJson, KEY_LORA_SECRET_KEY, cJSON_CreateString(g_deviceSettings.loraSecretKey));
    cJSON_AddItemToObject(rootJson, KEY_LORA_SPREADING_FACTOR, cJSON_CreateNumber(g_deviceSettings.loraSpreadingFactor));
    cJSON_AddItemToObject(rootJson, KEY_LORA_BANDWIDTH, cJSON_CreateNumber(g_deviceSettings.loraBandwidth));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_BROKER_HOST, cJSON_CreateString(g_deviceSettings.mqttBrokerHost));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_BROKER_PORT, cJSON_CreateNumber(g_deviceSettings.mqttBrokerPort));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_TLS_MODE, cJSON_CreateNumber(g_deviceSettings.mqttTlsMode));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_SUBSCRIBE_TOPIC, cJSON_CreateString(g_deviceSettings.mqttSubscribeTopic));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_SUBSCRIBE_QOS, cJSON_CreateNumber(g_deviceSettings.mqttSubscribeQos));
    cJSON_AddItemToObject(rootJson, KEY_MQTT_PUBLISH_TIMEOUT_MS, cJSON_CreateNumber(g_deviceSettings.mqttPublishTimeoutMs));
    retStr = cJSON_Print(rootJson);
    RemoveFormatChar(retStr);
    cJSON_Delete(rootJson);

    return retStr;
}

static void SetDefaultDeviceSettings(void)
{
    memset(&g_deviceSettings, 0, sizeof(g_deviceSettings));

    g_deviceSettings.brightness = DEFAULT_BRIGHTNESS;
    g_deviceSettings.loraChatAvatarColor = DEFAULT_LORA_CHAT_AVATAR_COLOR;
    CopyStringValue(g_deviceSettings.loraChatUsername, sizeof(g_deviceSettings.loraChatUsername), DEFAULT_LORA_CHAT_USERNAME);
    g_deviceSettings.mqttChatAvatarColor = DEFAULT_MQTT_CHAT_AVATAR_COLOR;
    CopyStringValue(g_deviceSettings.mqttChatUsername, sizeof(g_deviceSettings.mqttChatUsername), DEFAULT_MQTT_CHAT_USERNAME);
    g_deviceSettings.loraFreq = DEFAULT_LORA_FREQ;
    g_deviceSettings.deviceId = DEFAULT_DEVICE_ID;
    g_deviceSettings.loraNetId = DEFAULT_LORA_NET_ID;
    CopyStringValue(g_deviceSettings.loraSecretKey, sizeof(g_deviceSettings.loraSecretKey), DEFAULT_LORA_SECRET_KEY);
    g_deviceSettings.loraSpreadingFactor = DEFAULT_LORA_SPREADING_FACTOR;
    g_deviceSettings.loraBandwidth = DEFAULT_LORA_BANDWIDTH;
    CopyStringValue(g_deviceSettings.mqttBrokerHost, sizeof(g_deviceSettings.mqttBrokerHost), DEFAULT_MQTT_BROKER_HOST);
    g_deviceSettings.mqttBrokerPort = DEFAULT_MQTT_BROKER_PORT;
    g_deviceSettings.mqttTlsMode = DEFAULT_MQTT_TLS_MODE;
    CopyStringValue(g_deviceSettings.mqttSubscribeTopic, sizeof(g_deviceSettings.mqttSubscribeTopic), DEFAULT_MQTT_SUBSCRIBE_TOPIC);
    g_deviceSettings.mqttSubscribeQos = DEFAULT_MQTT_SUBSCRIBE_QOS;
    g_deviceSettings.mqttPublishTimeoutMs = DEFAULT_MQTT_PUBLISH_TIMEOUT_MS;
}

static void CopyStringValue(char *dst, size_t dstSize, const char *src)
{
    if (dst == NULL || dstSize == 0 || src == NULL) {
        return;
    }

    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
}
