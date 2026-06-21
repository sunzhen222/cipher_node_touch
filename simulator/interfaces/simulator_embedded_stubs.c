#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "background_task.h"
#include "drv_lcd.h"
#include "drv_w25qxx.h"
#include "flash_map.h"
#include "hardware_version.h"
#include "lora.h"
#include "mqtt.h"
#include "software_version.h"
#include "test_cmd.h"
#include "user_assert.h"
#include "user_memory.h"
#include "wifi_connect.h"
#include "wifi_search.h"

#ifndef SIMULATOR_FLASH_FILE
#define SIMULATOR_FLASH_FILE "simulator_flash.bin"
#endif

#define SIMULATOR_FLASH_SIZE (SPI_FLASH_ADDR_ERR_INFO + SPI_FLASH_SIZE_ERR_INFO)
#define SIMULATOR_FLASH_ERASE_VALUE 0xFF

static uint32_t g_lcdBrightness = 80;
static bool g_wifiConnected = false;
static bool g_mqttConnected = false;
static char g_wifiSsid[32] = "";

typedef struct {
    BackgroundAsyncFunc_t func;
    void *inData;
    uint32_t inDataLen;
    uint32_t delay;
} SimulatorAsync_t;

void ShowAssert(const char *file, uint32_t line);

static void ensure_flash_file(void);
static void flash_read(uint8_t *buffer, uint32_t addr, uint32_t size);
static void flash_write(const uint8_t *buffer, uint32_t addr, uint32_t size);
static void flash_fill(uint32_t addr, uint32_t size, uint8_t value);
static int async_thread(void *argument);

static void copy_string(char *dest, size_t destSize, const char *src)
{
    if (destSize == 0) {
        return;
    }
    snprintf(dest, destSize, "%s", src == NULL ? "" : src);
}

static void ensure_flash_file(void)
{
    FILE *file = fopen(SIMULATOR_FLASH_FILE, "r+b");
    uint8_t buffer[256];
    long size = 0;

    if (file == NULL) {
        file = fopen(SIMULATOR_FLASH_FILE, "w+b");
        if (file == NULL) {
            ShowAssert(__FILE__, __LINE__);
            abort();
        }
    } else {
        if (fseek(file, 0, SEEK_END) != 0) {
            fclose(file);
            ShowAssert(__FILE__, __LINE__);
            abort();
        }
        size = ftell(file);
        if (size < 0) {
            fclose(file);
            ShowAssert(__FILE__, __LINE__);
            abort();
        }
    }

    memset(buffer, SIMULATOR_FLASH_ERASE_VALUE, sizeof(buffer));
    while ((uint32_t)size < SIMULATOR_FLASH_SIZE) {
        uint32_t remaining = SIMULATOR_FLASH_SIZE - (uint32_t)size;
        size_t chunk = remaining > sizeof(buffer) ? sizeof(buffer) : remaining;
        if (fwrite(buffer, 1, chunk, file) != chunk) {
            fclose(file);
            ShowAssert(__FILE__, __LINE__);
            abort();
        }
        size += (long)chunk;
    }

    fclose(file);
}

static void flash_read(uint8_t *buffer, uint32_t addr, uint32_t size)
{
    FILE *file;

    if (size == 0) {
        return;
    }
    if (buffer == NULL || addr > SIMULATOR_FLASH_SIZE || size > SIMULATOR_FLASH_SIZE - addr) {
        ShowAssert(__FILE__, __LINE__);
        return;
    }

    ensure_flash_file();
    file = fopen(SIMULATOR_FLASH_FILE, "rb");
    if (file == NULL || fseek(file, (long)addr, SEEK_SET) != 0 || fread(buffer, 1, size, file) != size) {
        if (file != NULL) {
            fclose(file);
        }
        ShowAssert(__FILE__, __LINE__);
        return;
    }
    fclose(file);
}

static void flash_write(const uint8_t *buffer, uint32_t addr, uint32_t size)
{
    FILE *file;
    uint8_t *oldData;

    if (size == 0) {
        return;
    }
    if (buffer == NULL || addr > SIMULATOR_FLASH_SIZE || size > SIMULATOR_FLASH_SIZE - addr) {
        ShowAssert(__FILE__, __LINE__);
        return;
    }

    oldData = malloc(size);
    if (oldData == NULL) {
        ShowAssert(__FILE__, __LINE__);
        abort();
    }

    flash_read(oldData, addr, size);
    for (uint32_t i = 0; i < size; i++) {
        oldData[i] &= buffer[i];
    }

    file = fopen(SIMULATOR_FLASH_FILE, "r+b");
    if (file == NULL || fseek(file, (long)addr, SEEK_SET) != 0 || fwrite(oldData, 1, size, file) != size) {
        free(oldData);
        if (file != NULL) {
            fclose(file);
        }
        ShowAssert(__FILE__, __LINE__);
        return;
    }

    free(oldData);
    fclose(file);
}

static void flash_fill(uint32_t addr, uint32_t size, uint8_t value)
{
    FILE *file;
    uint8_t buffer[256];

    if (size == 0) {
        return;
    }
    if (addr > SIMULATOR_FLASH_SIZE || size > SIMULATOR_FLASH_SIZE - addr) {
        ShowAssert(__FILE__, __LINE__);
        return;
    }

    memset(buffer, value, sizeof(buffer));
    ensure_flash_file();
    file = fopen(SIMULATOR_FLASH_FILE, "r+b");
    if (file == NULL || fseek(file, (long)addr, SEEK_SET) != 0) {
        if (file != NULL) {
            fclose(file);
        }
        ShowAssert(__FILE__, __LINE__);
        return;
    }

    while (size > 0) {
        size_t chunk = size > sizeof(buffer) ? sizeof(buffer) : size;
        if (fwrite(buffer, 1, chunk, file) != chunk) {
            fclose(file);
            ShowAssert(__FILE__, __LINE__);
            return;
        }
        size -= (uint32_t)chunk;
    }

    fclose(file);
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

void RegisterTestCmd(const char *cmdString, const TestCmdFunc_t func) { (void)cmdString; (void)func; }
bool CompareAndRunTestCmd(const char *inputString) { (void)inputString; return false; }

bool W25qxx_Init(void)
{
    ensure_flash_file();
    return true;
}

void W25qxx_EraseChip(void)
{
    flash_fill(0, SIMULATOR_FLASH_SIZE, SIMULATOR_FLASH_ERASE_VALUE);
}

void W25qxx_EraseAddr(uint32_t addr)
{
    uint32_t sectorAddr = addr - (addr % FATFS_FLASH_SECTOR_SIZE);
    flash_fill(sectorAddr, FATFS_FLASH_SECTOR_SIZE, SIMULATOR_FLASH_ERASE_VALUE);
}

void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size)
{
    flash_write(pBuffer, addr, size);
}

void W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t addr, uint32_t size)
{
    flash_read(pBuffer, addr, size);
}

void SetLcdBackLight(uint32_t brightness) { g_lcdBrightness = brightness; }
bool LcdIsOpen(void) { return true; }
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
    SimulatorAsync_t *async;
    SDL_Thread *thread;

    if (func == NULL) {
        return;
    }

    async = malloc(sizeof(SimulatorAsync_t));
    if (async == NULL) {
        ShowAssert(__FILE__, __LINE__);
        return;
    }
    memset(async, 0, sizeof(SimulatorAsync_t));
    async->func = func;
    async->inDataLen = inDataLen;
    async->delay = delay;

    if (inData != NULL && inDataLen > 0) {
        async->inData = malloc(inDataLen);
        if (async->inData == NULL) {
            free(async);
            ShowAssert(__FILE__, __LINE__);
            return;
        }
        memcpy(async->inData, inData, inDataLen);
    }

    thread = SDL_CreateThread(async_thread, "background_async", async);
    if (thread == NULL) {
        free(async->inData);
        free(async);
        ShowAssert(__FILE__, __LINE__);
        return;
    }
    SDL_DetachThread(thread);
}

static int async_thread(void *argument)
{
    SimulatorAsync_t *async = argument;

    if (async->delay > 0) {
        SDL_Delay(async->delay);
    }
    async->func(async->inData, async->inDataLen);
    free(async->inData);
    free(async);
    return 0;
}
