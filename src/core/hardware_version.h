
#ifndef _HARDWARE_VERSION_H
#define _HARDWARE_VERSION_H

#include "stdint.h"
#include "stdbool.h"

enum {
    HW_VER_1_0 = 0,
    HW_VER_1_1,
    HW_VER_1_2,
    HW_VER_1_3,
    HW_VER_UNKNOWN,
};

uint32_t GetHardwareVersion(void);
const char *GetHardwareVersionString(void);

#endif
