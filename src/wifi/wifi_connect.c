
#include "wifi_connect.h"
#include "at_command.h"
#include "user_assert.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"


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

static bool SendAtCommandWaitOk(const char *command, uint32_t timeoutMs)
{
    ASSERT(command != NULL);

    bool ok = false;
    uint32_t elapsed = 0;
    char received[AT_COMMAND_MAX_LENGTH];

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand(command);

    while (elapsed < timeoutMs) {
        if (!GetReceivedAtCommand(received, 200)) {
            elapsed += 200;
            continue;
        }

        char trimmed[AT_COMMAND_MAX_LENGTH];
        strncpy(trimmed, received, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimLineEnd(trimmed);

        if (trimmed[0] == '\0') {
            continue;
        }

        if (strcmp(trimmed, "OK") == 0) {
            ok = true;
            break;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            ok = false;
            break;
        }
    }

    AtCommandUnlock();
    return ok;
}

bool ConnectWifi(const char *ssid, const char *password)
{
    ASSERT(ssid != NULL);
    ASSERT(password != NULL);

    bool connectOk = false;

    if (ssid[0] == '\0') {
        return false;
    }

    char command[AT_COMMAND_MAX_LENGTH];
    snprintf(command, sizeof(command), "AT+WJAP=%s,%s", ssid, password);

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand(command);

    uint32_t elapsed = 0;
    char received[AT_COMMAND_MAX_LENGTH];
    bool done = false;

    while (!done && elapsed < 15000) {
        if (!GetReceivedAtCommand(received, 200)) {
            elapsed += 200;
            continue;
        }

        char trimmed[AT_COMMAND_MAX_LENGTH];
        strncpy(trimmed, received, sizeof(trimmed) - 1);
        trimmed[sizeof(trimmed) - 1] = '\0';
        TrimLineEnd(trimmed);

        if (trimmed[0] == '\0') {
            continue;
        }

        if (strcmp(trimmed, "+EVENT:WIFI_GOT_IP") == 0) {
            connectOk = true;
            done = true;
            continue;
        }

        if (strncmp(trimmed, "+WJAP:", 6) == 0) {
            int joinStatus = atoi(trimmed + 6);
            if (joinStatus != 0) {
                connectOk = false;
                done = true;
            }
            continue;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            connectOk = false;
            done = true;
        }
    }

    AtCommandUnlock();
    return connectOk;
}

bool GetWifiConnectInfo(WifiConnectInfo_t *info)
{
    ASSERT(info != NULL);

    bool connected = false;

    memset(info, 0, sizeof(WifiConnectInfo_t));

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand("AT+STAINFO?");

    bool gotStatus = false;
    bool gotSecurity = false;
    bool gotTerminalResponse = false;
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
            connected = info->connected;
            gotTerminalResponse = true;
            break;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            info->connected = false;
            connected = false;
            gotTerminalResponse = true;
            break;
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

    if (!gotTerminalResponse) {
        if (gotStatus) {
            info->status = status;
        }
        if (gotSecurity) {
            info->security = security;
        }
        info->connected = (gotStatus && status == 3);
        connected = info->connected;
    }

    AtCommandUnlock();
    return connected;
}

bool GetWifiRssi(int8_t *rssi)
{
    ASSERT(rssi != NULL);

    bool gotRssi = false;
    bool gotTerminalResponse = false;
    int8_t parsedRssi = 0;
    char received[AT_COMMAND_MAX_LENGTH];

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand("AT+WRSSI");

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
            gotTerminalResponse = true;
            break;
        }

        if (strcmp(trimmed, "ERROR") == 0) {
            gotRssi = false;
            gotTerminalResponse = true;
            break;
        }

        if (strncmp(trimmed, "+WRSSI", 6) == 0) {
            char *separator = strchr(trimmed, ':');
            if (separator != NULL) {
                parsedRssi = (int8_t)atoi(separator + 1);
                gotRssi = true;
            }
        }
    }

    if (!gotTerminalResponse && gotRssi) {
        *rssi = parsedRssi;
    }

    AtCommandUnlock();
    return gotRssi;
}

bool SetWifiAutoConnect(bool enable)
{
    char command[AT_COMMAND_MAX_LENGTH];
    snprintf(command, sizeof(command), "AT+WAUTOCONN=%d", enable ? 1 : 0);

    return SendAtCommandWaitOk(command, 5000);
}

bool DisconnectWifi(void)
{
    return SendAtCommandWaitOk("AT+WDISCONNECT", 5000);
}


