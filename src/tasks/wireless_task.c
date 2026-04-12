#include "wireless_task.h"
#include "llcc68/driver_llcc68.h"
#include "llcc68/driver_llcc68_interface.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "user_msg.h"
#include "lora.h"
#include "cmsis_os2.h"
#include "drv_gpio.h"
#include "device_settings.h"

static void WirelessTask(void *pvParameter);

osThreadId_t g_wirelessTaskId;

void CreateWirelessTask(void)
{
    const osThreadAttr_t wirelessTaskAttributes = {
        .name = "WirelessTask",
        .priority = osPriorityRealtime7,
        .stack_size = 2048,
    };
    g_wirelessTaskId = osThreadNew(WirelessTask, NULL, &wirelessTaskAttributes);
}

static void WirelessTask(void *pvParameter)
{
    UNUSED(pvParameter);
    Message_t rcvMsg;
    osStatus_t ret;

    while (1) {
        ret = osMessageQueueGet(g_wirelessQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case WIRELESS_MSG_LORA_SEND: {
            //portENTER_CRITICAL();
            DoLoraSendData(rcvMsg.buffer, rcvMsg.length);
            //portEXIT_CRITICAL();
            osDelay(10);
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

