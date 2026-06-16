#include "command_lora_chat.h"
#include "stdio.h"
#include "string.h"
#include "protocol_codec.h"
#include "user_memory.h"
#include "lora_chat.h"
#include "lora.h"
#include "protocol_parse.h"
#include "ui_msg.h"
#include "user_utils.h"

#define TYPE_USERNAME           0x01
#define TYPE_TEXT               0x02
#define TYPE_AVATAR_COLOR       0x03

static void CopyTlvBytes(const Tlv_t *tlv, uint8_t *dst, uint32_t dstLen)
{
    uint32_t copyLen = tlv->length;

    if (copyLen > dstLen) {
        copyLen = dstLen;
    }

    if (tlv->pValue != NULL) {
        memcpy(dst, tlv->pValue, copyLen);
    } else {
        memcpy(dst, &tlv->value, copyLen);
    }
}

void CommandLoraChat(FrameHead_t *head, const uint8_t *tlvData)
{
    uint32_t count;
    Tlv_t tlvs[8] = {0};
    const Tlv_t *usernameTlv = NULL;
    const Tlv_t *textTlv = NULL;
    const Tlv_t *avatarColorTlv = NULL;
    char username[33] = {0};
    char *text = NULL;
    uint8_t avatarColor[3] = {0};
    uint32_t color;

    count = GetTlvFromData(tlvs, 8, tlvData, GetTlvLength(head));
    for (uint32_t i = 0; i < count; i++) {
        switch (tlvs[i].type) {
        case TYPE_USERNAME:
            if (usernameTlv != NULL) {
                printf("lora chat: duplicated username\n");
                return;
            }
            usernameTlv = &tlvs[i];
            break;
        case TYPE_TEXT:
            if (textTlv != NULL) {
                printf("lora chat: duplicated text\n");
                return;
            }
            textTlv = &tlvs[i];
            break;
        case TYPE_AVATAR_COLOR:
            if (avatarColorTlv != NULL) {
                printf("lora chat: duplicated avatar color\n");
                return;
            }
            avatarColorTlv = &tlvs[i];
            break;
        default:
            break;
        }
    }

    if (usernameTlv == NULL || textTlv == NULL || avatarColorTlv == NULL) {
        printf("lora chat: missing required tlv\n");
        return;
    }
    if (usernameTlv->length == 0 || textTlv->length == 0) {
        printf("lora chat: username/text empty\n");
        return;
    }
    if (avatarColorTlv->length != 3) {
        printf("lora chat: invalid avatar color length=%u\n", avatarColorTlv->length);
        return;
    }

    CopyTlvBytes(usernameTlv, (uint8_t *)username, sizeof(username) - 1);
    username[sizeof(username) - 1] = '\0';

    text = SRAM_MALLOC(textTlv->length + 1);
    CopyTlvBytes(textTlv, (uint8_t *)text, textTlv->length);
    text[textTlv->length] = '\0';

    CopyTlvBytes(avatarColorTlv, avatarColor, sizeof(avatarColor));
    color = ((uint32_t)avatarColor[0] << 16) | ((uint32_t)avatarColor[1] << 8) | avatarColor[2];

    LoraChatItem_t *newItem = AddChatItem(username, text, ProtocolGetCurrentRxRssi(), false, color);
    SendUiMsg(UI_MSG_CODE_LORA_CHAT_ITEM, &newItem, sizeof(newItem));
    SRAM_FREE(text);
}

void SendLoraChat(const char *username, const char *text, uint32_t avatarColor)
{
    FrameHead_t head = {0};
    Tlv_t tlvs[3] = {0};
    uint8_t avatarColorBytes[3] = {(uint8_t)(avatarColor >> 16), (uint8_t)(avatarColor >> 8), (uint8_t)avatarColor};
    size_t usernameLen;
    size_t textLen;
    uint8_t *sendData;
    uint32_t sendLen;

    if (username == NULL || text == NULL) {
        return;
    }

    usernameLen = strlen(username);
    textLen = strlen(text);
    if (usernameLen == 0 || textLen == 0) {
        return;
    }

    head.commandId = 0x01;
    tlvs[0].type = TYPE_USERNAME;
    tlvs[0].length = usernameLen;
    tlvs[0].pValue = (void *)username;

    tlvs[1].type = TYPE_TEXT;
    tlvs[1].length = textLen;
    tlvs[1].pValue = (void *)text;

    tlvs[2].type = TYPE_AVATAR_COLOR;
    tlvs[2].length = 3;
    tlvs[2].pValue = avatarColorBytes;

    sendData = BuildFrame(&head, tlvs, 3);
    sendLen = GetFrameTotalLength(&head, tlvs, 3);
    LoraSendData(sendData, sendLen);
    PrintArray("sendData", sendData, sendLen);
    SRAM_FREE(sendData);
}
