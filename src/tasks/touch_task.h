
#ifndef _TOUCH_TASK_H
#define _TOUCH_TASK_H

#include "stdint.h"
#include "stdbool.h"
#include "drv_touch.h"

void CreateTouchTask(void);
TouchStatus_t *GetTouchStatus(void);

#endif
