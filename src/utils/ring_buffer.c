#include "string.h"
#include "user_assert.h"
#include "ring_buffer.h"

static uint32_t RingBufferUsedSize(const RingBuffer_t *ringBuffer)
{
    if (ringBuffer->head >= ringBuffer->tail) {
        return ringBuffer->head - ringBuffer->tail;
    }
    return ringBuffer->size - (ringBuffer->tail - ringBuffer->head);
}

void RingBufferInit(RingBuffer_t *ringBuffer, uint8_t *buffer, uint32_t size)
{
    ASSERT(ringBuffer != NULL);
    ASSERT(buffer != NULL);
    ASSERT(size >= 2U);

    ringBuffer->buffer = buffer;
    ringBuffer->size = size;
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
}

bool RingBufferIsEmpty(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return ringBuffer->head == ringBuffer->tail;
}

uint32_t RingBufferGetUsedSize(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return RingBufferUsedSize(ringBuffer);
}

uint32_t RingBufferGetFreeSize(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return (ringBuffer->size - 1U) - RingBufferUsedSize(ringBuffer);
}

void RingBufferClear(RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
}

uint32_t RingBufferWrite(RingBuffer_t *ringBuffer, const uint8_t *data, uint32_t dataLen)
{
    uint32_t freeLen;
    uint32_t writeLen;
    uint32_t firstChunk;

    ASSERT(ringBuffer != NULL);
    ASSERT(data != NULL);

    if (dataLen == 0U) {
        return 0U;
    }

    freeLen = RingBufferGetFreeSize(ringBuffer);
    writeLen = (dataLen < freeLen) ? dataLen : freeLen;
    if (writeLen == 0U) {
        return 0U;
    }

    firstChunk = ringBuffer->size - ringBuffer->head;
    if (firstChunk > writeLen) {
        firstChunk = writeLen;
    }

    memcpy(&ringBuffer->buffer[ringBuffer->head], data, firstChunk);
    if (writeLen > firstChunk) {
        memcpy(&ringBuffer->buffer[0], &data[firstChunk], writeLen - firstChunk);
    }

    ringBuffer->head = (ringBuffer->head + writeLen) % ringBuffer->size;
    return writeLen;
}

uint32_t RingBufferRead(RingBuffer_t *ringBuffer, uint8_t *data, uint32_t dataLen)
{
    uint32_t usedLen;
    uint32_t readLen;
    uint32_t firstChunk;

    ASSERT(ringBuffer != NULL);
    ASSERT(data != NULL);

    if (dataLen == 0U) {
        return 0U;
    }

    usedLen = RingBufferGetUsedSize(ringBuffer);
    readLen = (dataLen < usedLen) ? dataLen : usedLen;
    if (readLen == 0U) {
        return 0U;
    }

    firstChunk = ringBuffer->size - ringBuffer->tail;
    if (firstChunk > readLen) {
        firstChunk = readLen;
    }

    memcpy(data, &ringBuffer->buffer[ringBuffer->tail], firstChunk);
    if (readLen > firstChunk) {
        memcpy(&data[firstChunk], &ringBuffer->buffer[0], readLen - firstChunk);
    }

    ringBuffer->tail = (ringBuffer->tail + readLen) % ringBuffer->size;
    return readLen;
}
