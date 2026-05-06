#include "protocol_parse.h"
#include "stdio.h"
#include "string.h"
#include "user_utils.h"
#include "protocol_codec.h"
#include "crc.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_utils.h"
#include "device_settings.h"
#include "command_lora_chat.h"
#include "drv_gpio.h"
#include "ctaes.h"
#include "sha256.h"

#define PROTOCOL_PARSE_OVERTIME             500

typedef struct {
    CommandCallbackFunc_t func;
    bool override;
} CommandItem_t;

static char g_iv[] = "c50e69d1f00dedb09869353f8771b9b4";

uint8_t g_protocolRcvBuffer[PROTOCOL_MAX_LENGTH];


static void ProtocolParse(uint8_t *data, uint16_t len);
static void ExecuteCommand(FrameHead_t *head, const uint8_t *tlvData);
static void BuildLoraAesKey(uint8_t key[SHA256_SIZE_BYTES]);


static const CommandItem_t g_CommandList[] = {
    {NULL,                  false},         //0
    {CommandLoraChat,      false},         //1
};


void ProtocolReceivedData(const uint8_t *data, uint32_t len)
{
    static uint32_t lastTick = 0;
    static uint16_t rcvLen = 0, rcvCount = 0;
    uint32_t tick, i;

    //PrintArray("rcv data", data, len);
    tick = osKernelGetTickCount();
    if (rcvCount != 0) {
        if (tick - lastTick > PROTOCOL_PARSE_OVERTIME) {
            printf("protocol over time, rcvCount=%u\n", rcvCount);
            PrintArray("g_protocolRcvBuffer", g_protocolRcvBuffer, rcvCount);
            rcvCount = 0;
            rcvLen = 0;
        }
    }
    lastTick = tick;

    for (i = 0; i < len; i++) {
        if (rcvCount >= PROTOCOL_MAX_LENGTH) {
            printf("protocol overflow, reset parser\n");
            rcvCount = 0;
            rcvLen = 0;
        }
        if (rcvCount == 0) {
            if (data[i] == PROTOCOL_HEADER) {
                g_protocolRcvBuffer[rcvCount] = data[i];
                rcvCount++;
            }
        } else if (rcvCount == 4) {
            //length
            g_protocolRcvBuffer[rcvCount] = data[i];
            rcvCount++;
            rcvLen = ((uint32_t)g_protocolRcvBuffer[4] << 8) + g_protocolRcvBuffer[3];
            if ((uint32_t)rcvLen + 11 > PROTOCOL_MAX_LENGTH) {
                printf("protocol length too large, rcvLen=%u\n", rcvLen);
                rcvCount = 0;
                rcvLen = 0;
            }
            //printf("rcvLen=%u\n", rcvLen);
        } else if (rcvCount == rcvLen + 10) {
            g_protocolRcvBuffer[rcvCount] = data[i];
            rcvCount = 0;
            //printf("full frame,len=%u\n", rcvLen + 11);
            ProtocolParse(g_protocolRcvBuffer, rcvLen + 11);
            rcvLen = 0;
        } else {
            g_protocolRcvBuffer[rcvCount] = data[i];
            rcvCount++;
        }
    }
}


static void BuildLoraAesKey(uint8_t key[SHA256_SIZE_BYTES])
{
    const char *secretKey = DeviceSettingsGetLoraSecretKey();
    if (secretKey == NULL) {
        secretKey = "";
    }
    sha256(secretKey, strlen(secretKey), key);
}


static void ProtocolParse(uint8_t *data, uint16_t len)
{
    FrameHead_t *pHead;
    uint16_t receivedCrc, calculatedCrc;
    uint32_t contentCrc;
    uint16_t serialIndex;
    static uint16_t lastSerialIndex = 0;

    //PrintArray("parse data", data, len);
    do {
        if (data == NULL || len < sizeof(FrameHead_t) + 2) {
            printf("invalid data\n");
            break;
        }
        pHead = (FrameHead_t *)data;
        if (pHead->head != PROTOCOL_HEADER) {
            printf("invalid head\n");
            break;
        }
        if (pHead->length + 11 != len) {
            printf("len err\n");
            break;
        }
        //printf("received frame:\n");
        //PrintFrameHead(pHead);
        memcpy(&receivedCrc, data + len - 2, 2);
        calculatedCrc = crc16_ccitt(data, len - 2);
        if (receivedCrc != calculatedCrc) {
            printf("crc err,receivedCrc=0x%04X,calculatedCrc=0x%04X\n", receivedCrc, calculatedCrc);
            break;
        }
        if (pHead->flag.b.networkId != DeviceSettingsGetLoraNetId()) {
            printf("net ID err,rcv=%u,local=%lu\n", pHead->flag.b.networkId, DeviceSettingsGetLoraNetId());
            break;
        }
        if (pHead->flag.b.encrypt != 0) {
            uint8_t key[SHA256_SIZE_BYTES] = {0};
            uint8_t *iv = StrToHex(g_iv);
            AES256_CBC_ctx ctx;
            uint32_t cryptLen = len - UNCIPHERED_HEADER_SIZE - 2;
            if (cryptLen % 16 != 0) {
                printf("cryptLen err\n");
                break;
            }
            uint8_t *decryptData = SRAM_MALLOC(cryptLen);
            BuildLoraAesKey(key);
            AES256_CBC_init(&ctx, key, iv);
            AES256_CBC_decrypt(&ctx, cryptLen / 16, decryptData, data + UNCIPHERED_HEADER_SIZE);
            memcpy(data + UNCIPHERED_HEADER_SIZE, decryptData, cryptLen);
            //PrintArray("decryptData", decryptData, cryptLen);
            SRAM_FREE(iv);
            SRAM_FREE(decryptData);
        }

        contentCrc = crc32_ieee(0, data + UNCIPHERED_HEADER_SIZE, pHead->length);
        if (contentCrc != pHead->contentCrc) {
            printf("content crc err,received=0x%08lX,calculated=0x%08lX\n", pHead->contentCrc, contentCrc);
            break;
        }

        serialIndex = pHead->serialIndex;
        if (serialIndex != lastSerialIndex + 1) {
            printf("not continuous serial index\n");
            printf("lastSerialIndex=%u,serialIndex=%u\n", lastSerialIndex, serialIndex);
        }
        lastSerialIndex = serialIndex;
        ExecuteCommand(pHead, data + sizeof(FrameHead_t));
    } while (0);

}


static void ExecuteCommand(FrameHead_t *head, const uint8_t *tlvData)
{
    uint32_t i;

    for (i = 0; i < sizeof(g_CommandList) / sizeof(g_CommandList[0]); i++) {
        if (head->commandId == i) {
            if (g_CommandList[i].override == false && head->flag.b.override != 0) {
                //received override frame, but not allowed.
                printf("override err\n");
                return;
            }
            if (g_CommandList[i].func == NULL) {
                printf("err, no func\n");
                return;
            }
            g_CommandList[i].func(head, tlvData);
        }
    }
    return;
}

