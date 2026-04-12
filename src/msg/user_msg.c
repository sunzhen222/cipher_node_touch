#include "user_msg.h"
#include "cmsis_os.h"
#include "general_msg.h"

osMessageQueueId_t g_cmdQueue;
osMessageQueueId_t g_usbQueue;
osMessageQueueId_t g_uiQueue;
osMessageQueueId_t g_backgroundQueue;
osMessageQueueId_t g_wirelessQueue;

void UserMsgInit(void)
{
    //Queues for the message mechanism are created here
    g_cmdQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_usbQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_uiQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_backgroundQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    g_wirelessQueue = osMessageQueueNew(10, sizeof(Message_t), NULL);
    //All messages are registered here
    SubMessageID(CMD_MSG_FRAME, g_cmdQueue);

    SubMessageID(USB_MSG_ISR_HANDLER, g_usbQueue);
    SubMessageID(USB_MSG_SEND_CDC, g_usbQueue);

    SubMessageID(UI_MSG_USER_EVENT, g_uiQueue);
    SubMessageID(UI_MSG_HOME, g_uiQueue);
    SubMessageID(UI_MSG_RELOAD_THEME, g_uiQueue);


    SubMessageID(BACKGROUND_MSG_SECOND, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_LORA_IRQ, g_backgroundQueue);
    SubMessageID(BACKGROUND_MSG_EXECUTE, g_backgroundQueue);

    SubMessageID(WIRELESS_MSG_LORA_SEND, g_wirelessQueue);
}
