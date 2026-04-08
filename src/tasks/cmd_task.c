#include "cmd_task.h"
#include "cmsis_os.h"
#include "general_msg.h"
#include "user_msg.h"
#include "test_cmd.h"
#include "user_utils.h"


static void CmdTask(void *pvParameter);


uint8_t g_testCmdRcvBuffer[256];
uint8_t g_testCmdRcvCount = 0;
osThreadId_t g_cmdTaskId;

void CreateCmdTask(void)
{
    const osThreadAttr_t cmdTaskAttributes = {
        .name = "CmdTask",
        .priority = osPriorityHigh7,
        .stack_size = 8192,
    };
    g_cmdTaskId = osThreadNew(CmdTask, NULL, &cmdTaskAttributes);
}


static void CmdTask(void *pvParameter)
{
    UNUSED(pvParameter);
    Message_t rcvMsg;
    osStatus_t ret;
    while (1) {
        ret = osMessageQueueGet(g_cmdQueue, &rcvMsg, NULL, osWaitForever);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case CMD_MSG_FRAME:
            CompareAndRunTestCmd((char *) g_testCmdRcvBuffer + 1);
            break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            vPortFree(rcvMsg.buffer);
        }
    }
}


void TestCmdRcvByte(uint8_t byte)
{
    static uint32_t lastTick = 0;
    uint32_t tick;

    if (osKernelGetState() < osKernelRunning) {
        return;
    }
    tick = osKernelGetTickCount();
    if (g_testCmdRcvCount != 0) {
        if (tick - lastTick > 200) {
            g_testCmdRcvCount = 0;
        }
    }
    lastTick = tick;

    if (g_testCmdRcvCount == 0) {
        if (byte == '#') {
            g_testCmdRcvBuffer[g_testCmdRcvCount] = byte;
            g_testCmdRcvCount++;
        }
    } else if (byte == '\n' || byte == '#') {
        g_testCmdRcvBuffer[g_testCmdRcvCount] = 0;
        if (g_testCmdRcvBuffer[g_testCmdRcvCount - 1] == '\r') {
            g_testCmdRcvBuffer[g_testCmdRcvCount - 1] = 0;
        }
        g_testCmdRcvCount = 0;
        PubValueMsg(CMD_MSG_FRAME, 0);
    } else if (g_testCmdRcvCount >= 254) {
        g_testCmdRcvCount = 0;
    } else {
        g_testCmdRcvBuffer[g_testCmdRcvCount] = byte;
        g_testCmdRcvCount++;
    }
}

