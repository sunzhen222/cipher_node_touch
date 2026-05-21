
#include "wifi_connect.h"
#include "at_command.h"
#include "user_assert.h"
#include "string.h"
#include "stdlib.h"


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


static bool ParseStaInfoSecurity(const char *line, WifiSecurityType *security)
{
    char parseBuffer[AT_COMMAND_MAX_LENGTH];
    char *token = NULL;
    char *securityStr = NULL;

    strncpy(parseBuffer, line, sizeof(parseBuffer) - 1);
    parseBuffer[sizeof(parseBuffer) - 1] = '\0';

    token = strtok(parseBuffer, ",");
    if (token == NULL) {
        return false;
    }

    token = strtok(NULL, ",");
    securityStr = token;
    if (securityStr == NULL) {
        return false;
    }

    *security = ParseSecurityType(securityStr);
    return true;
}

bool ConnectWifi(const char *ssid, const char *password)
{
    (void)ssid;
    (void)password;
    ClearReceivedAtCommand();
    return false;
}

bool GetWifiConnectInfo(WifiConnectInfo_t *info)
{
    ASSERT(info != NULL);

    memset(info, 0, sizeof(WifiConnectInfo_t));

    ClearReceivedAtCommand();
    SendAtCommand("AT+STAINFO?");

    bool gotStatus = false;
    bool gotSecurity = false;
    uint8_t status = 0;
    WifiSecurityType security = WIFI_SECURITY_UNKNOWN;
    char received[AT_COMMAND_MAX_LENGTH];

    while (GetReceivedAtCommand(received, 5000)) {
        char trimmed[AT_COMMAND_MAX_LENGTH];
        strncpy(trimmed, received, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimLineEnd(trimmed);

        if (trimmed[0] == '\0') {
            continue;
        }

        if (strcmp(trimmed, "OK") == 0) {
            if (gotStatus) {
                info->status = status;
            }
            if (gotSecurity) {
                info->security = security;
            }
            info->connected = (gotStatus && status == 3);
            return info->connected;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            info->connected = false;
            return info->connected;
        }

        if (strncmp(trimmed, "+STAINFO:", 9) == 0) {
            int parsedStatus = atoi(trimmed + 9);
            if (parsedStatus >= 0 && parsedStatus <= 255) {
                status = (uint8_t)parsedStatus;
            }
            gotStatus = true;
            continue;
        }

        if (strncmp(trimmed, "SSID:", 5) == 0) {
            strncpy(info->ssid, trimmed + 5, sizeof(info->ssid) - 1);
            info->ssid[sizeof(info->ssid) - 1] = '\0';
            continue;
        }

        if (strncmp(trimmed, "Password:", 9) == 0) {
            strncpy(info->password, trimmed + 9, sizeof(info->password) - 1);
            info->password[sizeof(info->password) - 1] = '\0';
            continue;
        }

        if (ParseStaInfoSecurity(trimmed, &security)) {
            gotSecurity = true;
            continue;
        }
    }

    if (gotStatus) {
        info->status = status;
    }
    if (gotSecurity) {
        info->security = security;
    }
    info->connected = (gotStatus && status == 3);
    return info->connected;
}

bool GetWifiRssi(int8_t *rssi)
{
    ASSERT(rssi != NULL);

    ClearReceivedAtCommand();
    SendAtCommand("AT+WRSSI");

    bool gotRssi = false;
    int8_t parsedRssi = 0;
    char received[AT_COMMAND_MAX_LENGTH];

    while (GetReceivedAtCommand(received, 5000)) {
        char trimmed[AT_COMMAND_MAX_LENGTH];
        strncpy(trimmed, received, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimLineEnd(trimmed);

        if (trimmed[0] == '\0') {
            continue;
        }

        if (strcmp(trimmed, "OK") == 0) {
            if (gotRssi) {
                *rssi = parsedRssi;
            }
            return gotRssi;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            return false;
        }

        if (strncmp(trimmed, "+WRSSI", 6) == 0) {
            char *separator = strchr(trimmed, ':');
            if (separator != NULL) {
                parsedRssi = (int8_t)atoi(separator + 1);
                gotRssi = true;
            }
        }
    }

    if (gotRssi) {
        *rssi = parsedRssi;
    }
    return gotRssi;
}


