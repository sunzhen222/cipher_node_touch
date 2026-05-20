
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
} WiFiSecurityType;

typedef struct WiFiItem_t {
    char ssid[32];
    uint8_t ch;
    WiFiSecurityType security;
    int8_t rssi;
    uint8_t bssid[6];
    struct WiFiItem_t *next;
} WiFiItem_t;

uint32_t SearchWiFi(WiFiItem_t *wifiListHead);
void FreeWiFiList(WiFiItem_t *wifiListHead);
const char *WiFiSecurityToString(WiFiSecurityType security);

#endif
