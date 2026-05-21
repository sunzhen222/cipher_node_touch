
#ifndef _RING_BUFFER_H
#define _RING_BUFFER_H

#include "stdint.h"
#include "stdbool.h"

typedef struct {
    uint8_t *buffer;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
} RingBuffer_t;

void RingBufferInit(RingBuffer_t *ringBuffer, uint8_t *buffer, uint32_t size);
bool RingBufferIsEmpty(const RingBuffer_t *ringBuffer);
uint32_t RingBufferGetUsedSize(const RingBuffer_t *ringBuffer);
uint32_t RingBufferGetFreeSize(const RingBuffer_t *ringBuffer);
void RingBufferClear(RingBuffer_t *ringBuffer);
uint32_t RingBufferWrite(RingBuffer_t *ringBuffer, const uint8_t *data, uint32_t dataLen);
uint32_t RingBufferRead(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t dataLen);

#endif
