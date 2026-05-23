
#ifndef _WIFI_CONNECT_H
#define _WIFI_CONNECT_H

#include "stdint.h"
#include "stdbool.h"
#include "wifi_search.h"

typedef struct {
    char ssid[32];
    char password[64];
    uint8_t status;
    WifiSecurityType security;
    bool connected;
} WifiConnectInfo_t;

bool ConnectWifi(const char *ssid, const char *password);
bool GetWifiConnectInfo(WifiConnectInfo_t *info);
bool GetWifiRssi(int8_t *rssi);
bool SetWifiAutoConnect(bool enable);
bool DisconnectWifi(void);

#endif
