
#ifndef _LORA_CHAT_H
#define _LORA_CHAT_H

#include "stdint.h"
#include "stdbool.h"

typedef struct ChatItem_t {
    char name[16];
    char *text;
    uint8_t rssi;
    struct ChatItem_t *next;
} ChatItem_t;

void LoraChatInit(void);
void ClearChatItems(void);
void AddChatItem(const char *name, const char *text, uint8_t rssi);
void StartGetChatItem(void);
ChatItem_t *GetNextChatItem(void);

void TestLoraChat(void);

#endif
