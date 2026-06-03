
#ifndef _AT_COMMAND_H
#define _AT_COMMAND_H

#include "stdint.h"
#include "stdbool.h"

#define AT_COMMAND_MAX_LENGTH       256

void AtCommandInit(void);
void AtCommandByteReceived(uint8_t byte);
void AtCommandLock(void);
void AtCommandUnlock(void);
void ClearReceivedAtCommand(void);
bool HasReceivedAtCommandData(void);
bool GetReceivedAtCommand(char *buffer, uint32_t timeout);
void ProcessAtCommand(void);
void SendAtCommand(const char *cmd);
void TrimLineEnd(char *str);
bool SendAtCommandWait(const char *command,
                       const char *expectResult,
                       const char *errResult,
                       uint32_t timeout);

#endif
