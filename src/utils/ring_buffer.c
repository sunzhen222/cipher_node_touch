#include "string.h"
#include "user_assert.h"
#include "ring_buffer.h"

/**
 * @brief Get the number of bytes currently stored in the ring buffer.
 * @param ringBuffer Ring buffer instance.
 * @return Number of used bytes.
 */
static uint32_t RingBufferUsedSize(const RingBuffer_t *ringBuffer)
{
    if (ringBuffer->head >= ringBuffer->tail) {
        return ringBuffer->head - ringBuffer->tail;
    }
    return ringBuffer->size - (ringBuffer->tail - ringBuffer->head);
}

/**
 * @brief Initialize a ring buffer instance with caller-provided storage.
 * @param ringBuffer Ring buffer instance to initialize.
 * @param buffer Raw storage used by the ring buffer.
 * @param size Raw storage size in bytes. Usable capacity is size - 1.
 */
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

/**
 * @brief Check whether the ring buffer contains no data.
 * @param ringBuffer Ring buffer instance.
 * @return true if empty, otherwise false.
 */
bool RingBufferIsEmpty(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return ringBuffer->head == ringBuffer->tail;
}

/**
 * @brief Get the number of bytes currently available to read.
 * @param ringBuffer Ring buffer instance.
 * @return Number of readable bytes.
 */
uint32_t RingBufferGetUsedSize(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return RingBufferUsedSize(ringBuffer);
}

/**
 * @brief Get the number of bytes that can still be written.
 * @param ringBuffer Ring buffer instance.
 * @return Number of writable bytes.
 */
uint32_t RingBufferGetFreeSize(const RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    return (ringBuffer->size - 1U) - RingBufferUsedSize(ringBuffer);
}

/**
 * @brief Discard all data in the ring buffer.
 * @param ringBuffer Ring buffer instance.
 */
void RingBufferClear(RingBuffer_t *ringBuffer)
{
    ASSERT(ringBuffer != NULL);
    ringBuffer->head = 0;
    ringBuffer->tail = 0;
}

/**
 * @brief Write bytes into the ring buffer.
 * @param ringBuffer Ring buffer instance.
 * @param data Source data buffer.
 * @param dataLen Number of bytes requested to write.
 * @return Number of bytes actually written. This may be less than dataLen when
 *         free space is insufficient.
 */
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

/**
 * @brief Read bytes from the ring buffer.
 * @param ringBuffer Ring buffer instance.
 * @param data Destination data buffer.
 * @param dataLen Number of bytes requested to read.
 * @return Number of bytes actually read. This may be less than dataLen when
 *         stored data is insufficient.
 */
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
