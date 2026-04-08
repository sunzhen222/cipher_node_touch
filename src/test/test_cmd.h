#ifndef _TEST_CMD_H
#define _TEST_CMD_H

#include "stdint.h"
#include "stdbool.h"

typedef void (*TestCmdFunc_t)(int argc, char *argv[]);

void RegisterTestCmd(const char *cmdString, const TestCmdFunc_t func);
bool CompareAndRunTestCmd(const char *inputString);


#endif



