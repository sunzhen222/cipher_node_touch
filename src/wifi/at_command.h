
#ifndef _AT_COMMAND_H
#define _AT_COMMAND_H

#include "stdint.h"
#include "stdbool.h"

void AtCommandByteReceived(uint8_t byte);
void ProcessAtCommand(void);
void SendAtCommand(const char *cmd);

#endif
