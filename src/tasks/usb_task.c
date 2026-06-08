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
#include "ring_buffer.h"

#define USB_IRQ_IN_TASK                 1
#define USB_CDC_RING_BUFFER_LEN         1536
#define USB_CDC_TX_BUFF_LEN             256

typedef enum {
    USB_CDC_TX_IDLE = 0,
    USB_CDC_TX_READY,
    USB_CDC_TX_ACTIVE,
} UsbCdcTxState_t;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static void UsbTask(void *pvParameter);
static void UsbTestFunc(int argc, char *argv[]);
static void UsbCdcSendFromRingBuffer(void);

osThreadId_t g_usbTaskId;
static RingBuffer_t g_usbCdcRingBuffer;
static uint8_t *g_usbCdcRingRawBuffer;
static uint8_t *g_usbCdcTxBuff;
static uint32_t g_usbCdcTxLen;
static volatile UsbCdcTxState_t g_usbCdcTxState = USB_CDC_TX_IDLE;

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
    g_usbCdcRingRawBuffer = SRAM_MALLOC(USB_CDC_RING_BUFFER_LEN + 1);
    g_usbCdcTxBuff = SRAM_MALLOC(USB_CDC_TX_BUFF_LEN);
    RingBufferInit(&g_usbCdcRingBuffer, g_usbCdcRingRawBuffer, USB_CDC_RING_BUFFER_LEN + 1);
    MX_USB_DEVICE_Init();
    while (1) {
        ret = osMessageQueueGet(g_usbQueue, &rcvMsg, NULL,
                                (g_usbCdcTxState == USB_CDC_TX_READY) ? 1 : 10000);
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
                UsbCdcSendFromRingBuffer();
            }
            break;
            default:
                break;
            }
            if (rcvMsg.buffer != NULL) {
                SRAM_FREE(rcvMsg.buffer);
            }
        }
        if (g_usbCdcTxState != USB_CDC_TX_ACTIVE) {
            UsbCdcSendFromRingBuffer();
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
    ASSERT(data != NULL);
    if (g_usbCdcRingRawBuffer == NULL) {
        return;
    }

    if (len == 0) {
        return;
    }

    __disable_irq();
    RingBufferWrite(&g_usbCdcRingBuffer, data, len);
    __enable_irq();

    PubValueMsg(USB_MSG_SEND_CDC, 0);
}

void UsbCdcTransmitComplete(void)
{
    g_usbCdcTxState = USB_CDC_TX_IDLE;
    g_usbCdcTxLen = 0;
    PubValueMsg(USB_MSG_SEND_CDC, 0);
}

static void UsbCdcSendFromRingBuffer(void)
{
    uint8_t sendRet;

    if (g_usbCdcTxState == USB_CDC_TX_ACTIVE) {
        return;
    }

    if (g_usbCdcTxState == USB_CDC_TX_IDLE) {
        ASSERT(g_usbCdcTxBuff != NULL);
        __disable_irq();
        g_usbCdcTxLen = RingBufferRead(&g_usbCdcRingBuffer, g_usbCdcTxBuff, USB_CDC_TX_BUFF_LEN);
        __enable_irq();

        if (g_usbCdcTxLen == 0) {
            return;
        }
        g_usbCdcTxState = USB_CDC_TX_READY;
    }

    sendRet = CDC_Transmit_FS(g_usbCdcTxBuff, g_usbCdcTxLen);
    if (sendRet == USBD_OK) {
        g_usbCdcTxState = USB_CDC_TX_ACTIVE;
    } else if (sendRet != USBD_BUSY) {
        printf("usb cdc send err,%d\n", sendRet);
        g_usbCdcTxLen = 0;
        g_usbCdcTxState = USB_CDC_TX_IDLE;
        PubValueMsg(USB_MSG_SEND_CDC, 0);
    }
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
