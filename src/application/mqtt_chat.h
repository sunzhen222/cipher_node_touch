#ifndef _MQTT_CHAT_H
#define _MQTT_CHAT_H

#include "stdint.h"
#include "stdbool.h"

typedef struct MqttChatItem_t {
    char name[16];
    char *text;
    bool self;
    uint32_t headColor;
    struct MqttChatItem_t *next;
} MqttChatItem_t;

void MqttChatInit(void);
void ClearMqttChatItems(void);
MqttChatItem_t *AddMqttChatItem(const char *name,
                                const char *text,
                                bool self,
                                uint32_t headColor);
void StartGetMqttChatItem(void);
MqttChatItem_t *GetNextMqttChatItem(void);
bool ProcessMqttChatAtCommand(const char *received);

void TestMqttChat(void);

#endif
