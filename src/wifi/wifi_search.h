
#ifndef _SEARCH_WIFI_H
#define _SEARCH_WIFI_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    WIFI_SECURITY_OPEN = 0,
    WIFI_SECURITY_WEP,
    WIFI_SECURITY_WPA,
    WIFI_SECURITY_WPA2,
    WIFI_SECURITY_UNKNOWN,
} WifiSecurityType;

typedef struct WifiItem_t {
    char ssid[32];
    uint8_t ch;
    WifiSecurityType security;
    int8_t rssi;
    uint8_t bssid[6];
    struct WifiItem_t *next;
} WifiItem_t;

uint32_t SearchWifi(WifiItem_t *wifiListHead);
void FreeWifiList(WifiItem_t *wifiListHead);
const char *WifiSecurityToString(WifiSecurityType security);

#endif
