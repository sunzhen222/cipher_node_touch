#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "background_task.h"
#include "device_settings.h"
#include "drv_lcd.h"
#include "drv_w25qxx.h"
#include "hardware_version.h"
#include "lora.h"
#include "mqtt.h"
#include "software_version.h"
#include "user_assert.h"
#include "user_memory.h"
#include "wifi_connect.h"
#include "wifi_search.h"

static uint32_t g_brightness = 80;
static uint32_t g_lockScreenTime = 30;
static uint32_t g_loraAvatarColor = 0x2FB35A;
static uint32_t g_mqttAvatarColor = 0x6155F5;
static uint32_t g_loraFreq = 438000000U;
static uint32_t g_deviceId = 0x12345678U;
static uint32_t g_loraNetId = 1;
static uint32_t g_loraSf = LLCC68_LORA_SF_7;
static uint32_t g_loraBw = LLCC68_LORA_BANDWIDTH_250_KHZ;
static uint32_t g_mqttBrokerPort = 1883;
static uint32_t g_mqttTlsMode = 0;
static uint32_t g_mqttSubscribeQos = 0;
static uint32_t g_mqttPublishTimeoutMs = 3000;
static bool g_wifiConnected = false;
static bool g_mqttConnected = false;
static char g_loraUsername[32] = "Alice";
static char g_mqttUsername[32] = "MQTT User";
static char g_loraSecretKey[96] = "simulator-secret";
static char g_mqttBrokerHost[64] = "broker.emqx.io";
static char g_mqttSubscribeTopic[64] = "cipher_node_touch/simulator";
static char g_wifiSsid[32] = "";

static void copy_string(char *dest, size_t destSize, const char *src)
{
    if (destSize == 0) {
        return;
    }
    snprintf(dest, destSize, "%s", src == NULL ? "" : src);
}

void ShowAssert(const char *file, uint32_t line)
{
    fprintf(stderr, "ASSERT: %s:%lu\n", file, (unsigned long)line);
}

void *SramMallocTrack(size_t size, const char *file, int line, const char *func)
{
    void *p = malloc(size);
    (void)file;
    (void)line;
    (void)func;
    if (p == NULL) {
        ShowAssert(__FILE__, __LINE__);
        abort();
    }
    return p;
}

void SramFreeTrack(void *p, const char *file, int line, const char *func)
{
    (void)file;
    (void)line;
    (void)func;
    free(p);
}

void *SramReallocTrack(void *p, size_t size, const char *file, int line, const char *func)
{
    void *newPtr = realloc(p, size);
    (void)file;
    (void)line;
    (void)func;
    if (newPtr == NULL) {
        ShowAssert(__FILE__, __LINE__);
        abort();
    }
    return newPtr;
}

void *SramMalloc(size_t size) { return SramMallocTrack(size, __FILE__, __LINE__, __func__); }
void SramFree(void *p) { SramFreeTrack(p, __FILE__, __LINE__, __func__); }
void *SramRealloc(void *p, size_t size) { return SramReallocTrack(p, size, __FILE__, __LINE__, __func__); }
void PrintHeapInfo(void) {}

void DeviceSettingsInit(void) {}
void SaveDeviceSettings(void) { printf("simulator: SaveDeviceSettings\n"); }
uint32_t DeviceSettingsGetBrightness(void) { return g_brightness; }
void DeviceSettingsSetBrightness(uint32_t brightness) { g_brightness = brightness; }
uint32_t DeviceSettingsGetLoraChatAvatarColor(void) { return g_loraAvatarColor; }
void DeviceSettingsSetLoraChatAvatarColor(uint32_t color) { g_loraAvatarColor = color; }
const char *DeviceSettingsGetLoraChatUsername(void) { return g_loraUsername; }
void DeviceSettingsSetLoraChatUsername(const char *username) { copy_string(g_loraUsername, sizeof(g_loraUsername), username); }
uint32_t DeviceSettingsGetMqttChatAvatarColor(void) { return g_mqttAvatarColor; }
void DeviceSettingsSetMqttChatAvatarColor(uint32_t color) { g_mqttAvatarColor = color; }
const char *DeviceSettingsGetMqttChatUsername(void) { return g_mqttUsername; }
void DeviceSettingsSetMqttChatUsername(const char *username) { copy_string(g_mqttUsername, sizeof(g_mqttUsername), username); }
uint32_t DeviceSettingsGetLoraFreq(void) { return g_loraFreq; }
void DeviceSettingsSetLoraFreq(uint32_t freq) { g_loraFreq = freq; }
uint32_t DeviceSettingsGetDeviceId(void) { return g_deviceId; }
void DeviceSettingsSetDeviceId(uint32_t id) { g_deviceId = id; }
uint32_t DeviceSettingsGetLoraNetId(void) { return g_loraNetId; }
void DeviceSettingsSetLoraNetId(uint32_t id) { g_loraNetId = id; }
const char *DeviceSettingsGetLoraSecretKey(void) { return g_loraSecretKey; }
void DeviceSettingsSetLoraSecretKey(const char *secretKey) { copy_string(g_loraSecretKey, sizeof(g_loraSecretKey), secretKey); }
uint32_t DeviceSettingsGetLoraSpreadingFactor(void) { return g_loraSf; }
void DeviceSettingsSetLoraSpreadingFactor(uint32_t sf) { g_loraSf = sf; }
uint32_t DeviceSettingsGetLoraBandwidth(void) { return g_loraBw; }
void DeviceSettingsSetLoraBandwidth(uint32_t bw) { g_loraBw = bw; }
const char *DeviceSettingsGetMqttBrokerHost(void) { return g_mqttBrokerHost; }
void DeviceSettingsSetMqttBrokerHost(const char *host) { copy_string(g_mqttBrokerHost, sizeof(g_mqttBrokerHost), host); }
uint32_t DeviceSettingsGetMqttBrokerPort(void) { return g_mqttBrokerPort; }
void DeviceSettingsSetMqttBrokerPort(uint32_t port) { g_mqttBrokerPort = port; }
uint32_t DeviceSettingsGetMqttTlsMode(void) { return g_mqttTlsMode; }
void DeviceSettingsSetMqttTlsMode(uint32_t tlsMode) { g_mqttTlsMode = tlsMode; }
const char *DeviceSettingsGetMqttSubscribeTopic(void) { return g_mqttSubscribeTopic; }
void DeviceSettingsSetMqttSubscribeTopic(const char *topic) { copy_string(g_mqttSubscribeTopic, sizeof(g_mqttSubscribeTopic), topic); }
uint32_t DeviceSettingsGetMqttSubscribeQos(void) { return g_mqttSubscribeQos; }
void DeviceSettingsSetMqttSubscribeQos(uint32_t qos) { g_mqttSubscribeQos = qos; }
uint32_t DeviceSettingsGetMqttPublishTimeoutMs(void) { return g_mqttPublishTimeoutMs; }
void DeviceSettingsSetMqttPublishTimeoutMs(uint32_t timeoutMs) { g_mqttPublishTimeoutMs = timeoutMs; }
uint32_t DeviceSettingsGetLockScreenTime(void) { return g_lockScreenTime; }
void DeviceSettingsSetLockScreenTime(uint32_t lockScreenTime) { g_lockScreenTime = lockScreenTime; }
void PrintDeviceSettings(void) {}
void DeviceSettingsTest(int argc, char *argv[]) { (void)argc; (void)argv; }

void SetLcdBackLight(uint32_t brightness) { g_brightness = brightness; }
bool LcdIsOpen(void) { return true; }
void W25qxx_EraseChip(void) { printf("simulator: W25qxx_EraseChip\n"); }
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size) { (void)pBuffer; (void)addr; (void)size; }
void NVIC_SystemReset(void) { printf("simulator: NVIC_SystemReset\n"); }

uint32_t GetHardwareVersion(void) { return 0; }
const char *GetHardwareVersionString(void) { return "simulator"; }
const char *GetSoftwareVersionString(void) { return "simulator"; }
const char *GetBuildTime(void) { return __DATE__ " " __TIME__; }

uint8_t LoraSettings(void) { printf("simulator: LoraSettings\n"); return 0; }
const char *LoraGetWirelessStatus(void) { return "simulator"; }
bool LoraTxBusy(void) { return false; }
void SendLoraChat(const char *username, const char *text, uint32_t avatarColor)
{
    printf("simulator: SendLoraChat name=%s color=0x%06lX text=%s\n",
           username == NULL ? "" : username,
           (unsigned long)(avatarColor & 0xFFFFFF),
           text == NULL ? "" : text);
}

uint32_t SearchWifi(WifiItem_t *wifiListHead)
{
    WifiItem_t *node1;
    WifiItem_t *node2;

    if (wifiListHead == NULL) {
        return 0;
    }

    copy_string(wifiListHead->ssid, sizeof(wifiListHead->ssid), "Cipher Lab");
    wifiListHead->ch = 6;
    wifiListHead->security = WIFI_SECURITY_WPA2;
    wifiListHead->rssi = -42;

    node1 = SRAM_MALLOC(sizeof(WifiItem_t));
    memset(node1, 0, sizeof(WifiItem_t));
    copy_string(node1->ssid, sizeof(node1->ssid), "Open Simulator");
    node1->ch = 11;
    node1->security = WIFI_SECURITY_OPEN;
    node1->rssi = -66;

    node2 = SRAM_MALLOC(sizeof(WifiItem_t));
    memset(node2, 0, sizeof(WifiItem_t));
    copy_string(node2->ssid, sizeof(node2->ssid), "Workshop");
    node2->ch = 1;
    node2->security = WIFI_SECURITY_WPA2;
    node2->rssi = -82;

    wifiListHead->next = node1;
    node1->next = node2;
    return 3;
}

void FreeWifiList(WifiItem_t *wifiListHead)
{
    WifiItem_t *node;

    if (wifiListHead == NULL) {
        return;
    }

    node = wifiListHead->next;
    wifiListHead->next = NULL;
    while (node != NULL) {
        WifiItem_t *next = node->next;
        SRAM_FREE(node);
        node = next;
    }
}

const char *WifiSecurityToString(WifiSecurityType security)
{
    switch (security) {
    case WIFI_SECURITY_OPEN: return "OPEN";
    case WIFI_SECURITY_WEP: return "WEP";
    case WIFI_SECURITY_WPA: return "WPA";
    case WIFI_SECURITY_WPA2: return "WPA2";
    default: return "UNKNOWN";
    }
}

bool ConnectWifi(const char *ssid, const char *password)
{
    (void)password;
    copy_string(g_wifiSsid, sizeof(g_wifiSsid), ssid);
    g_wifiConnected = true;
    return true;
}

bool GetWifiConnectInfo(WifiConnectInfo_t *info)
{
    if (info == NULL) {
        return false;
    }
    memset(info, 0, sizeof(*info));
    copy_string(info->ssid, sizeof(info->ssid), g_wifiSsid);
    info->security = WIFI_SECURITY_WPA2;
    info->connected = g_wifiConnected;
    return true;
}

bool GetWifiRssi(int8_t *rssi)
{
    if (rssi != NULL) {
        *rssi = -48;
    }
    return true;
}

bool SetWifiAutoConnect(bool enable) { (void)enable; return true; }
bool DisconnectWifi(void) { g_wifiConnected = false; g_wifiSsid[0] = '\0'; return true; }

int32_t ConnectMqtt(void) { g_mqttConnected = true; return MQTT_CONNECT_OK; }
bool IsMqttConnected(void) { return g_mqttConnected; }
bool DisconnectMqtt(void) { g_mqttConnected = false; return true; }
bool PublishMqtt(const char *topic, uint8_t qos, bool retained, const char *payload)
{
    printf("simulator: PublishMqtt topic=%s qos=%u retained=%u payload=%s\n",
           topic == NULL ? "" : topic,
           qos,
           retained ? 1U : 0U,
           payload == NULL ? "" : payload);
    return true;
}
void GetMqttSenderId(char *buffer, size_t bufferSize) { copy_string(buffer, bufferSize, "SIM00001"); }

void CreateBackgroundTask(void) {}
bool InBackgroundTask(void) { return true; }
void AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen, uint32_t delay)
{
    void *copy = NULL;
    (void)delay;

    if (func == NULL) {
        return;
    }

    if (inData != NULL && inDataLen > 0) {
        copy = malloc(inDataLen);
        if (copy == NULL) {
            ShowAssert(__FILE__, __LINE__);
            return;
        }
        memcpy(copy, inData, inDataLen);
    }
    func(copy, inDataLen);
    free(copy);
}
