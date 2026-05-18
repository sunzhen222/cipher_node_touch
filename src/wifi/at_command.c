#include "at_command.h"
#include "cmsis_os2.h"
#include "stdio.h"
#include "string.h"
#include "user_msg.h"
#include "user_assert.h"
#include "user_utils.h"
#include "drv_uart.h"

#define AT_COMMAND_TIMEOUT_MS    100

static uint8_t g_atCommandBuffer[256];
static uint8_t g_atCommandPacket[256];
static uint32_t g_atCommandIndex = 0;

void AtCommandByteReceived(uint8_t byte)
{
    static uint32_t lastTick = 0;
    uint32_t tick = osKernelGetTickCount();

    if ((tick - lastTick) > AT_COMMAND_TIMEOUT_MS) {
        g_atCommandIndex = 0;
    }
    lastTick = tick;
    if (g_atCommandIndex > sizeof(g_atCommandBuffer) - 2) {
        g_atCommandIndex = sizeof(g_atCommandBuffer) - 2;
    }
    g_atCommandBuffer[g_atCommandIndex] = byte;
    g_atCommandIndex++;

    if (byte == '\n') {
        g_atCommandBuffer[g_atCommandIndex] = '\0';
        strncpy((char *)g_atCommandPacket, (char *)g_atCommandBuffer, sizeof(g_atCommandPacket) - 1);
        g_atCommandPacket[g_atCommandIndex] = '\0';
        g_atCommandIndex = 0;
        PubValueMsg(BACKGROUND_MSG_AT_COMMAND, 0);
    }
}

void ProcessAtCommand(void)
{
    printf("Received AT command: %s", g_atCommandPacket);
}

void SendAtCommand(const char *cmd)
{
    ASSERT(cmd != NULL);
    uint32_t len = strlen(cmd);
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, len, 1000);
    char newline[2] = "\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t *)newline, 2, 1000);
}

