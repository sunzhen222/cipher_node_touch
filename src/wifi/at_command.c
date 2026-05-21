#include "at_command.h"
#include "cmsis_os2.h"
#include "stdio.h"
#include "string.h"
#include "user_assert.h"
#include "ring_buffer.h"
#include "drv_uart.h"

#define AT_COMMAND_RING_SIZE     (AT_COMMAND_MAX_LENGTH * 4)

static uint8_t g_atCommandRingRawBuffer[AT_COMMAND_RING_SIZE];
static RingBuffer_t g_atCommandRingBuffer;

void AtCommandInit(void)
{
    RingBufferInit(&g_atCommandRingBuffer, g_atCommandRingRawBuffer, sizeof(g_atCommandRingRawBuffer));
}

void AtCommandByteReceived(uint8_t byte)
{
    __disable_irq();
    RingBufferWrite(&g_atCommandRingBuffer, &byte, 1);
    __enable_irq();
}

void ClearReceivedAtCommand(void)
{
    __disable_irq();
    RingBufferClear(&g_atCommandRingBuffer);
    __enable_irq();
}

bool GetReceivedAtCommand(char *buffer, uint32_t timeout)
{
    ASSERT(buffer != NULL);

    uint32_t tick = 0;
    uint32_t index = 0;
    bool overflow = false;

    while (1) {
        uint8_t byte = 0;
        bool hasData = false;

        __disable_irq();
        if (RingBufferGetUsedSize(&g_atCommandRingBuffer) > 0) {
            RingBufferRead(&g_atCommandRingBuffer, &byte, 1);
            hasData = true;
        }
        __enable_irq();

        if (hasData) {
            if (!overflow && index < (AT_COMMAND_MAX_LENGTH - 1)) {
                buffer[index] = (char)byte;
                index++;
            } else {
                overflow = true;
            }

            if (byte == '\n') {
                if (index >= (AT_COMMAND_MAX_LENGTH - 1)) {
                    index = AT_COMMAND_MAX_LENGTH - 1;
                }
                buffer[index] = '\0';
                return true;
            }

            continue;
        }

        if (tick >= timeout) {
            break;
        }

        osDelay(1);
        tick++;
    }

    buffer[0] = '\0';
    return false;
}

void ProcessAtCommand(void)
{
    char received[AT_COMMAND_MAX_LENGTH];
    if (GetReceivedAtCommand(received, 0)) {
        printf("Received AT command: %s", received);
    }
}

void SendAtCommand(const char *cmd)
{
    ASSERT(cmd != NULL);
    uint32_t len = strlen(cmd);
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, len, 1000);
    char newline[2] = "\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)newline, 2, 1000);
}

void TrimLineEnd(char *str)
{
    ASSERT(str != NULL);

    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
        str[len - 1] = '\0';
        len--;
    }
}

