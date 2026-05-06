#ifndef _PROTOCOL_CODEC_H
#define _PROTOCOL_CODEC_H

#include "stdint.h"
#include "stdbool.h"

#define PROTOCOL_MAX_LENGTH         1024
#define PROTOCOL_HEADER             0x4C


#define UNCIPHERED_HEADER_SIZE      9

typedef union {
    struct {
        uint16_t networkId      : 12;   // Network identifier, using the remaining bits
        uint16_t override       : 1;    // Skip the anti-replay check
        uint16_t ack            : 1;    // Acknowledgment bit
        uint16_t reserved       : 1;    // Reserved bit
        uint16_t encrypt        : 1;    // Encryption enabled bit
    } b;
    uint16_t u16;                       // Full 2-byte representation
} FrameHeadFlag_t;

#pragma pack(1)
typedef struct {
    uint8_t head;
    FrameHeadFlag_t flag;
    uint16_t length;
    uint32_t contentCrc;
    uint32_t timeStamp;
    uint16_t serialIndex;
    uint8_t commandId;
} FrameHead_t;
#pragma pack()

typedef struct {
    uint8_t type;
    uint16_t length;
    void *pValue;
    uint32_t value;
} Tlv_t;

uint8_t *BuildFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen);
uint32_t GetFrameTotalLength(const FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen);
uint32_t GetTlvFromFrame(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen);
uint32_t GetTlvFromData(Tlv_t tlvArray[], uint32_t maxTlvLen, const uint8_t *data, uint32_t dataLen);
uint32_t GetTlvLength(const FrameHead_t *head);
void PrintProtocolFrame(FrameHead_t *pHead, const Tlv_t tlvArray[], uint32_t tlvLen);
void PrintFrameHead(const FrameHead_t *pHead);

void ProtocolCodecTest(int argc, char *argv[]);

#endif
