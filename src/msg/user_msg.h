#ifndef _USER_MSG_H
#define _USER_MSG_H

#include "stdint.h"
#include "stdbool.h"
#ifndef SIMULATOR
#include "cmsis_os.h"
#endif
#include "general_msg.h"

//GENERAL_MSG_BASE is an unclassified message, usually a broadcast message or a special message.
#define GENERAL_MSG_BASE                0x00000001
#define CMD_MSG_BASE                    0x00010000
#define USB_MSG_BASE                    0x00020000
#define UI_MSG_BASE                     0x00030000
#define BACKGROUND_MSG_BASE             0x00040000
#define WIRELESS_MSG_BASE               0x00050000

enum {
    CMD_MSG_FRAME = CMD_MSG_BASE,
};

enum {
    USB_MSG_ISR_HANDLER = USB_MSG_BASE,
    USB_MSG_SEND_CDC,
};

enum {
    UI_MSG_USER_EVENT = UI_MSG_BASE,
    UI_MSG_HOME,
    UI_MSG_RELOAD_THEME,
};

enum {
    BACKGROUND_MSG_SECOND = BACKGROUND_MSG_BASE,
    BACKGROUND_MSG_LORA_IRQ,
    BACKGROUND_MSG_EXECUTE,
};

enum {
    WIRELESS_MSG_LORA_SEND = WIRELESS_MSG_BASE,
};

extern osMessageQueueId_t g_cmdQueue;
extern osMessageQueueId_t g_usbQueue;
extern osMessageQueueId_t g_uiQueue;
extern osMessageQueueId_t g_backgroundQueue;
extern osMessageQueueId_t g_wirelessQueue;

void UserMsgInit(void);

#endif
