#ifndef _PROTOCOL_PARSE_H
#define _PROTOCOL_PARSE_H

#include "stdint.h"
#include "stdbool.h"
#include "protocol_codec.h"

enum {
    COMMAND_ID_LORA_CHAT            = 1,
    COMMAND_ID_MAX
};


#define TYPE_GENERAL_RESULT_ACK             0xFF


typedef void (*CommandCallbackFunc_t)(FrameHead_t *head, const uint8_t *tlvData);

void ProtocolReceivedData(const uint8_t *data, uint32_t len);
void ProtocolSetCurrentRxRssi(int8_t rssi);
int8_t ProtocolGetCurrentRxRssi(void);

#endif
