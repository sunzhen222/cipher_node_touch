#include "usb_task.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "user_memory.h"
#include "user_msg.h"
#include "user_assert.h"
#include "usb_device.h"
#include "test_cmd.h"
#include "usbd_cdc_if.h"

#define USB_IRQ_IN_TASK                 1
#define USB_CDC_BUFF_LEN                1024

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static void UsbTask(void *pvParameter);
static void UsbTestFunc(int argc, char *argv[]);

osThreadId_t g_usbTaskId;
static uint8_t g_usbCdcSendBuff[USB_CDC_BUFF_LEN];

void CreateUsbTask(void)
{
    const osThreadAttr_t cmdTaskAttributes = {
        .name = "UsbTask",
        .priority = osPriorityNormal,
        .stack_size = 8192,
    };
    g_usbTaskId = osThreadNew(UsbTask, NULL, &cmdTaskAttributes);
}

static void UsbTask(void *pvParameter)
{
    UNUSED(pvParameter);
    Message_t rcvMsg;
    osStatus_t ret;

    RegisterTestCmd("usb:", UsbTestFunc);
    MX_USB_DEVICE_Init();
    while (1) {
        ret = osMessageQueueGet(g_usbQueue, &rcvMsg, NULL, 10000);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case USB_MSG_ISR_HANDLER: {
#if (USB_IRQ_IN_TASK == 1)
                HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
                NVIC_ClearPendingIRQ(OTG_FS_IRQn);
                NVIC_EnableIRQ(OTG_FS_IRQn);
#endif
            }
            break;
            case USB_MSG_SEND_CDC: {
                uint8_t sendRet;
                sendRet = CDC_Transmit_FS(g_usbCdcSendBuff, rcvMsg.value);
                if (sendRet != 0) {
                    printf("usb cdc send err,%d\n", sendRet);
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
}


void OTG_FS_IRQHandler(void)
{
#if (USB_IRQ_IN_TASK == 1)
    NVIC_DisableIRQ(OTG_FS_IRQn);
    PubValueMsg(USB_MSG_ISR_HANDLER, 0);
#else
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
#endif
}


void SendUsbCdc(void *data, uint32_t len)
{
    ASSERT(len <= USB_CDC_BUFF_LEN);
    memcpy(g_usbCdcSendBuff, data, len);
    PubValueMsg(USB_MSG_SEND_CDC, len);
}


static void UsbTestFunc(int argc, char *argv[])
{
    if (argc < 1) {
        printf("usb test argc err,%d\n", argc);
        return;
    }
    if (strcmp(argv[0], "send_str") == 0) {
        VALUE_CHECK(argc, 2);
        SendUsbCdc(argv[1], strlen(argv[1]));
    }
}
