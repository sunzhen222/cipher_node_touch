
#ifndef _AT_COMMAND_H
#define _AT_COMMAND_H

#include "stdint.h"
#include "stdbool.h"

#define AT_COMMAND_MAX_LENGTH       256

void AtCommandInit(void);
void AtCommandByteReceived(uint8_t byte);
void ClearReceivedAtCommand(void);
bool GetReceivedAtCommand(char *buffer, uint32_t timeout);
void ProcessAtCommand(void);
void SendAtCommand(const char *cmd);
void TrimLineEnd(char *str);

#endif
