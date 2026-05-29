
#ifndef _LORA_CHAT_H
#define _LORA_CHAT_H

#include "stdint.h"
#include "stdbool.h"

typedef struct LoraChatItem_t {
    char name[16];
    char *text;
    uint8_t rssi;
    bool self;
    uint32_t headColor;
    struct LoraChatItem_t *next;
} LoraChatItem_t;

void LoraChatInit(void);
void ClearChatItems(void);
LoraChatItem_t *AddChatItem(const char *name, const char *text, uint8_t rssi, bool self, uint32_t headColor);
void StartGetChatItem(void);
LoraChatItem_t *GetNextChatItem(void);

void TestLoraChat(void);

#endif
