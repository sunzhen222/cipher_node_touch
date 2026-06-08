
#ifndef _USB_TASK_H
#define _USB_TASK_H

#include "stdint.h"
#include "stdbool.h"

void CreateUsbTask(void);
void UsbCdcRingBufferInit(void);
void SendUsbCdc(void *data, uint32_t len);
void UsbCdcTransmitComplete(void);

#endif
