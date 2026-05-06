#ifndef _COMMAND_LORA_CHAT_H
#define _COMMAND_LORA_CHAT_H

#include "stdint.h"
#include "stdbool.h"
#include "protocol_codec.h"

void CommandLoraChat(FrameHead_t *head, const uint8_t *tlvData);
void SendLoraChat(const char *username, const char *text, uint32_t avatarColor);

#endif
