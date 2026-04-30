#include "background_task.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h"
#include "user_memory.h"
#include "user_msg.h"
#include "user_utils.h"
#include "ui_msg.h"
#include "lora.h"
#include "battery.h"

typedef struct {
    BackgroundAsyncFunc_t func;
    void *inData;
    uint32_t inDataLen;
    uint32_t delay;
    osTimerId_t timer;
} BackgroundAsync_t;

static void BackgroundTask(void *argument);
static void SecondTickTimerFunc(void *argument);
static void AsyncTimerFunc(void *argument);

osThreadId_t g_backgroundTaskHandle;
osTimerId_t g_secondTickTimer;

void CreateBackgroundTask(void)
{
    const osThreadAttr_t backgroundTask_attributes = {
        .name = "BackgroundTask",
        .stack_size = 1024 * 12,
        .priority = (osPriority_t)osPriorityNormal,
    };
    g_backgroundTaskHandle = osThreadNew(BackgroundTask, NULL, &backgroundTask_attributes);
    g_secondTickTimer = osTimerNew(SecondTickTimerFunc, osTimerPeriodic, NULL, NULL);
    osTimerStart(g_secondTickTimer, 1000);
}

/// @brief Notify the background task to execute a function.
/// @param[in] func The function which would be executed in background task.
/// @param[in] inData.
/// @param[in] inDataLen.
/// @param[in] delay The delay time to execute the function.
/// @return None.
void AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen, uint32_t delay)
{
    BackgroundAsync_t *async = SRAM_MALLOC(sizeof(BackgroundAsync_t));
    memset(async, 0, sizeof(BackgroundAsync_t));
    async->func = func;
    if (inDataLen > 0) {
        async->inData = SRAM_MALLOC(inDataLen);
        memcpy(async->inData, inData, inDataLen);
        async->inDataLen = inDataLen;
    }
    async->delay = delay;
    async->timer = osTimerNew(AsyncTimerFunc, osTimerOnce, async, NULL);
    osTimerStart(async->timer, async->delay > 0 ? async->delay : 1);
}

bool InBackgroundTask(void)
{
    return osThreadGetId() == g_backgroundTaskHandle;
}

static void BackgroundTask(void *argument)
{
    UNUSED(argument);
    Message_t rcvMsg;
    osStatus_t ret;
    printf("device started\n");
    while (1) {
        ret = osMessageQueueGet(g_backgroundQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case BACKGROUND_MSG_SECOND: {
        }
        break;
        case BACKGROUND_MSG_REFRESH_BATTERY: {
            SendBatteryInfoToUi();
        }
        break;
        case BACKGROUND_MSG_LORA_IRQ: {
            LoraIrqHandler();
        }
        break;
        case BACKGROUND_MSG_EXECUTE: {
            if (rcvMsg.buffer == NULL || rcvMsg.length != sizeof(BackgroundAsync_t)) {
                printf("rcv BACKGROUND_MSG_EXECUTE err\n");
                break;
            }
            BackgroundAsync_t *async = (BackgroundAsync_t *)rcvMsg.buffer;
            if (async->func) {
                async->func(async->inData, async->inDataLen);
            }
            if (async->inData) {
                SRAM_FREE(async->inData);
            }
        }
        break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}

static void SecondTickTimerFunc(void *argument)
{
    UNUSED(argument);
    static uint32_t secondCount = 0;
    PubValueMsg(BACKGROUND_MSG_SECOND, 0);
    if (secondCount % 60 == 0) {
        PubValueMsg(BACKGROUND_MSG_REFRESH_BATTERY, 0);
    }
    secondCount++;
}

static void AsyncTimerFunc(void *argument)
{
    BackgroundAsync_t *async = argument;
    PubBufferMsg(BACKGROUND_MSG_EXECUTE, async, sizeof(BackgroundAsync_t));
    osTimerDelete(async->timer);
    SRAM_FREE(async);
}
