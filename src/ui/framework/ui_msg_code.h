#ifndef _UI_MSG_CODE_H
#define _UI_MSG_CODE_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    bool connected;
    int8_t rssi;
} UiMsgWifiStatus_t;


enum {
    UI_MSG_CODE_BATTERY_PERCENT = 0,
    UI_MSG_CODE_CHARGING,
    UI_MSG_CODE_SHT30,
    UI_MSG_CODE_WIFI,
    UI_MSG_CODE_FILE_SYSTEM_REFRESH,
    UI_MSG_CODE_LORA_CHAT_ITEM,
    UI_MSG_CODE_WIFI_SEARCH_RESULT,
    UI_MSG_CODE_WIFI_CONNECT_RESULT,
    UI_MSG_CODE_WIFI_DISCONNECT_RESULT,
};


#endif
