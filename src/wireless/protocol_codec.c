#include "protocol_codec.h"
#include "stdio.h"
#include "string.h"
#include "user_memory.h"
#include "user_utils.h"
#include "user_assert.h"
#include "assert.h"
#include "crc.h"
#include "drv_trng.h"
#include "device_settings.h"
#include "ctaes.h"
#include "sha256.h"
#include "cmsis_os2.h"

static char g_iv[] = "c50e69d1f00dedb09869353f8771b9b4";

static void BuildLoraAesKey(uint8_t key[SHA256_SIZE_BYTES])
{
    const char *secretKey = DeviceSettingsGetLoraSecretKey();
    if (secretKey == NULL) {
        secretKey = "";
    }
    sha256(secretKey, strlen(secretKey), key);
}

/// @brief Build frame.
/// @param tlvArray
/// @param tlvLen
/// @return full frame data which needs be freed manually.
uint8_t *BuildFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i, index, crc16Calc, contentCrc, timestamp;
    uint8_t *sendData;
    static uint16_t serialIndex = 0;

    if (serialIndex == 0) {
        serialIndex = TrngGetRandomNumber();
        printf("init serialIndex=%u\n", serialIndex);
    }
    pHead->flag.b.encrypt = 1;
    timestamp = osKernelGetTickCount();
    totalLen = GetFrameTotalLength(pHead, tlvArray, tlvLen);
    pHead->head = PROTOCOL_HEADER;
    pHead->flag.b.networkId = DeviceSettingsGetLoraNetId();
    pHead->length = totalLen - 11;
    pHead->contentCrc = 0;
    pHead->timeStamp = timestamp;
    pHead->serialIndex = serialIndex;
    serialIndex++;
    //printf("totalLen=%d\n", totalLen);
    sendData = SRAM_MALLOC(totalLen);
    memcpy(sendData, pHead, sizeof(FrameHead_t));
    index = sizeof(FrameHead_t);
    for (i = 0; i < tlvLen; i++) {
        //t
        sendData[index++] = tlvArray[i].type;
        //l
        ASSERT(tlvArray[i].length <= 0x7FFF);
        if (tlvArray[i].length > 127) {
            sendData[index++] = 0x80 | (tlvArray[i].length >> 8);
            sendData[index++] = tlvArray[i].length & 0xFF;
        } else {
            sendData[index++] = tlvArray[i].length;
        }
        //v
        if (tlvArray[i].pValue == NULL) {
            ASSERT(tlvArray[i].length <= 4);
            memcpy(&sendData[index], &tlvArray[i].value, tlvArray[i].length);
        } else {
            memcpy(&sendData[index], tlvArray[i].pValue, tlvArray[i].length);
        }
        index += tlvArray[i].length;
    }
    if (pHead->flag.b.encrypt != 0) {
        AES256_CBC_ctx ctx;
        uint32_t cryptLen = totalLen - UNCIPHERED_HEADER_SIZE - 2;
        uint8_t key[SHA256_SIZE_BYTES] = {0};
        ASSERT(cryptLen % 16 == 0);
        memset(sendData + index, 0xFF, totalLen - index - 2);
        uint8_t *iv = StrToHex(g_iv);
        uint8_t *encryptData = SRAM_MALLOC(cryptLen);
        BuildLoraAesKey(key);
        AES256_CBC_init(&ctx, key, iv);

        contentCrc = crc32_ieee(0, sendData + UNCIPHERED_HEADER_SIZE, pHead->length);
        pHead->contentCrc = contentCrc;
        ((FrameHead_t *)sendData)->contentCrc = contentCrc;

        AES256_CBC_encrypt(&ctx, cryptLen / 16, encryptData, sendData + UNCIPHERED_HEADER_SIZE);
        memcpy(sendData + UNCIPHERED_HEADER_SIZE, encryptData, cryptLen);
        SRAM_FREE(iv);
        SRAM_FREE(encryptData);

        //decrypt test
        //uint8_t *decryptData = SRAM_MALLOC(cryptLen);
        //BuildLoraAesKey(key);
        //iv = StrToHex(g_iv);
        //AES256_CBC_init(&ctx, key, iv);
        //AES256_CBC_decrypt(&ctx, cryptLen / 16, decryptData, sendData + UNCIPHERED_HEADER_SIZE);
        //PrintArray("decryptData", decryptData, cryptLen);
        //SRAM_FREE(iv);
        //SRAM_FREE(decryptData);
    } else {
        contentCrc = crc32_ieee(0, sendData + UNCIPHERED_HEADER_SIZE, pHead->length);
        pHead->contentCrc = contentCrc;
        ((FrameHead_t *)sendData)->contentCrc = contentCrc;
    }

    crc16Calc = crc16_ccitt(sendData, totalLen - 2);
    memcpy(&sendData[totalLen - 2], &crc16Calc, 2);
    //PrintArray("sendData", sendData, totalLen);
    return sendData;
}

/// @brief Get the frame total length by tlv array.
/// @param tlvArray
/// @return The frame total length.
uint32_t GetFrameTotalLength(const FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i;

    //totalLen = sizeof(FrameHead_t);                         //HEAD
    totalLen = 0;
    for (i = 0; i < tlvLen; i++) {
        totalLen++;                                         //t
        totalLen += tlvArray[i].length > 127 ? 2 : 1;       //l
        totalLen += tlvArray[i].length;                     //v
    }
    if (pHead->flag.b.encrypt != 0) {
        totalLen = GET_ALIGN(totalLen + (sizeof(FrameHead_t) - UNCIPHERED_HEADER_SIZE), 16) + UNCIPHERED_HEADER_SIZE;
    } else {
        totalLen += sizeof(FrameHead_t);
    }
    totalLen += 2;                                           //CRC16

    return totalLen;
}

/// @brief Get TLV array from data.
/// @param[out] tlvArray
/// @param[in] maxTlvLen
/// @param[in] data
/// @param[in] dataLen
/// @return TLV array number.
uint32_t GetTlvFromData(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen)
{
    uint32_t count, index = 0;

    for (count = 0; count < maxTlvLen; count++) {
        if (data[index] == 0xFF) {
            //0xFF padding
            break;
        }
        tlvArray[count].type = data[index++];
        if (data[index] > 127) {
            tlvArray[count].length = ((uint16_t)(data[index] & 0x7F) << 8) + data[index + 1];
            index += 2;
        } else {
            tlvArray[count].length = data[index];
            index++;
        }
        if (tlvArray[count].length + index > dataLen) {
            printf("invalid length,%u\n", tlvArray[count].length);
            break;
        }
        if (tlvArray[count].length == 1 || tlvArray[count].length == 2 || tlvArray[count].length == 4) {
            //tlvArray[count].value = *(uint32_t *)&data[index];
            memcpy(&tlvArray[count].value, &data[index], tlvArray[count].length);
        } else {
            tlvArray[count].pValue = (uint8_t *)&data[index];
        }
        index += tlvArray[count].length;
        ASSERT(index <= dataLen);
        //printf("type=%d\n", tlvArray[count].type);
        //PrintArray("v", tlvArray[count].pValue, tlvArray[count].length);
        if (index == dataLen) {
            //printf("TLV complete\n");
            count++;
            break;
        }
    }
    return count;
}

uint32_t GetTlvLength(const FrameHead_t *head)
{
    return head->length - 7;
}

uint32_t GetTlvFromFrame(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen)
{
    const FrameHead_t *head;
    uint16_t crc16Calc, crc16Read;
    uint32_t contentCrcCalc;

    if (data == NULL) {
        return 0;
    }
    if (dataLen < sizeof(FrameHead_t) + 2) {
        printf("dataLen err\n");
        return 0;
    }

    head = (const FrameHead_t *)data;
    if (head->head != PROTOCOL_HEADER) {
        printf("head err\n");
        return 0;
    }

    if ((uint32_t)head->length + 11 != dataLen) {
        printf("length err\n");
        return 0;
    }

    if (head->flag.b.encrypt != 0) {
        printf("frame encrypted, tlv unavailable before decrypt\n");
        return 0;
    }

    crc16Calc = crc16_ccitt(data, dataLen - 2);
    crc16Read = *(uint16_t *)&data[dataLen - 2];
    if (crc16Calc != crc16Read) {
        printf("crc err,%X,%X\n", crc16Calc, crc16Read);
        return 0;
    }

    contentCrcCalc = crc32_ieee(0, data + UNCIPHERED_HEADER_SIZE, head->length);
    if (contentCrcCalc != head->contentCrc) {
        printf("content crc err,calc=0x%08lX,read=0x%08lX\n", contentCrcCalc, head->contentCrc);
        return 0;
    }

    return GetTlvFromData(tlvArray, maxTlvLen, data + sizeof(FrameHead_t), GetTlvLength(head));
}

void PrintProtocolFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen)
{
    uint32_t totalLen, i, index, crc16Calc;
    uint8_t *sendData;

    totalLen = sizeof(FrameHead_t);                         //HEAD
    for (i = 0; i < tlvLen; i++) {
        totalLen++;                                         //t
        totalLen += tlvArray[i].length > 127 ? 2 : 1;       //l
        totalLen += tlvArray[i].length;                     //v
    }
    totalLen += 2;                                           //CRC16
    pHead->length = totalLen - 11;
    printf("totalLen=%lu\n", totalLen);
    sendData = SRAM_MALLOC(totalLen);
    memcpy(sendData, pHead, sizeof(FrameHead_t));
    index = sizeof(FrameHead_t);
    for (i = 0; i < tlvLen; i++) {
        //t
        sendData[index++] = tlvArray[i].type;
        //l
        ASSERT(tlvArray[i].length <= 0x7FFF);
        if (tlvArray[i].length > 127) {
            sendData[index++] = 0x80 | (tlvArray[i].length >> 8);
            sendData[index++] = tlvArray[i].length & 0xFF;
        } else {
            sendData[index++] = tlvArray[i].length;
        }
        //v
        if (tlvArray[i].pValue == NULL) {
            ASSERT(tlvArray[i].length <= 4);
            memcpy(&sendData[index], &tlvArray[i].value, totalLen - index);
        } else {
            memcpy(&sendData[index], tlvArray[i].pValue, totalLen - index);
        }
        index += tlvArray[i].length;
    }

    pHead->contentCrc = crc32_ieee(0, sendData + UNCIPHERED_HEADER_SIZE, pHead->length);
    ((FrameHead_t *)sendData)->contentCrc = pHead->contentCrc;

    crc16Calc = crc16_ccitt(sendData, totalLen - 2);
    printf("crc16Calc=0x%lX\n", crc16Calc);
    memcpy(&sendData[index], &crc16Calc, 2);
    PrintArray("sendData", sendData, totalLen);
    SRAM_FREE(sendData);
}

void PrintFrameHead(const FrameHead_t *pHead)
{
    printf("head=0x%X\n", pHead->head);
    printf("flag=0x%X\n", pHead->flag.u16);
    printf("networkId=%u\n", pHead->flag.b.networkId);
    printf("ack=%u\n", pHead->flag.b.ack);
    printf("encrypt=%u\n", pHead->flag.b.encrypt);
    printf("length=%u\n", pHead->length);
    printf("contentCrc=0x%08lX\n", pHead->contentCrc);
    printf("timeStamp=%lu\n", pHead->timeStamp);
    printf("serialIndex=%u\n", pHead->serialIndex);
    printf("commandId=%u\n", pHead->commandId);
}

void ProtocolCodecTest(int argc, char *argv[])
{
    FrameHead_t head = {0};
    Tlv_t tlvArray[5] = {0};

    if (argc == 0) {
        return;
    }
    if (strcmp(argv[0], "build") == 0) {
        head.head = PROTOCOL_HEADER;
        head.length = 1234;
        tlvArray[0].type = 1;
        tlvArray[0].length = strlen("HELLO") + 1;
        tlvArray[0].pValue = "HELLO";

        tlvArray[1].type = 2;
        tlvArray[1].length = strlen("WORLD") + 1;
        tlvArray[1].pValue = "WORLD";

        PrintProtocolFrame(&head, tlvArray, 2);
    }
}
