#ifndef _BACKGROUND_TASK_H
#define _BACKGROUND_TASK_H

#include "stdint.h"
#include "stdbool.h"

typedef int32_t (*BackgroundAsyncFunc_t)(const void *inData, uint32_t inDataLen);

void CreateBackgroundTask(void);
void AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen, uint32_t delay);
bool InBackgroundTask(void);

#endif
