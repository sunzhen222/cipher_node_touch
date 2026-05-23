
#include "wifi_search.h"
#include "at_command.h"
#include "drv_uart.h"
#include "user_memory.h"
#include "user_assert.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


static WifiSecurityType ParseSecurityType(const char *securityStr)
{
    if (strstr(securityStr, "WPA2") != NULL) {
        return WIFI_SECURITY_WPA2;
    }
    if (strstr(securityStr, "WPA") != NULL) {
        return WIFI_SECURITY_WPA;
    }
    if (strstr(securityStr, "WEP") != NULL) {
        return WIFI_SECURITY_WEP;
    }
    if (strstr(securityStr, "Open") != NULL) {
        return WIFI_SECURITY_OPEN;
    }
    return WIFI_SECURITY_UNKNOWN;
}


static void CopySsidWithFallback(char *dst, size_t dstSize, const char *src)
{
    size_t i = 0;

    if (dstSize == 0) {
        return;
    }

    while (src[i] != '\0' && i < dstSize - 1) {
        uint8_t c = (uint8_t)src[i];
        if (c >= 32 && c <= 126) {
            dst[i] = (char)c;
        } else {
            // Replace non-ASCII bytes with a visible placeholder.
            dst[i] = '?';
        }
        i++;
    }
    dst[i] = '\0';
}


static bool ParseBssid(const char *str, uint8_t out[6])
{
    unsigned int b0 = 0;
    unsigned int b1 = 0;
    unsigned int b2 = 0;
    unsigned int b3 = 0;
    unsigned int b4 = 0;
    unsigned int b5 = 0;
    int matched = sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &b0, &b1, &b2, &b3, &b4, &b5);

    if (matched != 6) {
        return false;
    }

    out[0] = (uint8_t)b0;
    out[1] = (uint8_t)b1;
    out[2] = (uint8_t)b2;
    out[3] = (uint8_t)b3;
    out[4] = (uint8_t)b4;
    out[5] = (uint8_t)b5;
    return true;
}


static bool ParseScanLine(const char *line, WifiItem_t *item)
{
    char parseBuffer[AT_COMMAND_MAX_LENGTH];
    char *token = NULL;
    char *indexStr = NULL;
    char *ssidStr = NULL;
    char *chStr = NULL;
    char *securityStr = NULL;
    char *rssiStr = NULL;
    char *bssidStr = NULL;

    strncpy(parseBuffer, line, sizeof(parseBuffer) - 1);
    parseBuffer[sizeof(parseBuffer) - 1] = '\0';
    TrimLineEnd(parseBuffer);

    if (parseBuffer[0] == '\0') {
        return false;
    }
    if (strstr(parseBuffer, "+WSCAN:") == parseBuffer) {
        return false;
    }

    token = strtok(parseBuffer, ",");
    indexStr = token;
    token = strtok(NULL, ",");
    ssidStr = token;
    token = strtok(NULL, ",");
    chStr = token;
    token = strtok(NULL, ",");
    securityStr = token;
    token = strtok(NULL, ",");
    rssiStr = token;
    token = strtok(NULL, ",");
    bssidStr = token;

    if (indexStr == NULL || ssidStr == NULL || chStr == NULL || securityStr == NULL || rssiStr == NULL || bssidStr == NULL) {
        return false;
    }

    memset(item, 0, sizeof(WifiItem_t));

    CopySsidWithFallback(item->ssid, sizeof(item->ssid), ssidStr);
    item->ch = (uint8_t)atoi(chStr);
    item->security = ParseSecurityType(securityStr);
    item->rssi = (int8_t)atoi(rssiStr);
    if (!ParseBssid(bssidStr, item->bssid)) {
        memset(item->bssid, 0, sizeof(item->bssid));
    }

    item->next = NULL;
    return true;
}

uint32_t SearchWifi(WifiItem_t *wifiListHead)
{
    ASSERT(wifiListHead != NULL);
    uint32_t count = 0;

    FreeWifiList(wifiListHead);
    memset(wifiListHead, 0, sizeof(WifiItem_t));

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand("AT+WSCAN");

    char received[AT_COMMAND_MAX_LENGTH];
    bool headFilled = false;
    WifiItem_t *tail = wifiListHead;

    while (GetReceivedAtCommand(received, 5000)) {
        //printf("%s", received);

        char trimmed[AT_COMMAND_MAX_LENGTH];
        strncpy(trimmed, received, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimLineEnd(trimmed);
        if (strcmp(trimmed, "OK") == 0) {
            break;
        }

        WifiItem_t parsed = {0};
        if (ParseScanLine(received, &parsed)) {
            if (!headFilled) {
                memcpy(wifiListHead, &parsed, sizeof(WifiItem_t));
                wifiListHead->next = NULL;
                tail = wifiListHead;
                headFilled = true;
                count++;
            } else {
                WifiItem_t *item = SRAM_MALLOC(sizeof(WifiItem_t));
                memcpy(item, &parsed, sizeof(WifiItem_t));
                item->next = NULL;
                tail->next = item;
                tail = item;
                count++;
            }
        }
    }

    AtCommandUnlock();
    return count;
}

void FreeWifiList(WifiItem_t *wifiListHead)
{
    ASSERT(wifiListHead != NULL);

    WifiItem_t *node = wifiListHead->next;
    while (node != NULL) {
        WifiItem_t *next = node->next;
        SRAM_FREE(node);
        node = next;
    }
    memset(wifiListHead, 0, sizeof(WifiItem_t));
}

const char *WifiSecurityToString(WifiSecurityType security)
{
    switch (security) {
    case WIFI_SECURITY_OPEN:
        return "OPEN";
    case WIFI_SECURITY_WEP:
        return "WEP";
    case WIFI_SECURITY_WPA:
        return "WPA";
    case WIFI_SECURITY_WPA2:
        return "WPA2";
    default:
        return "UNKNOWN";
    }
}

