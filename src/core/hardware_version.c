#include "hardware_version.h"
#include "drv_adc.h"

uint32_t GetHardwareVersion(void)
{
    static bool init = false;
    static uint32_t hardwareVersion = HW_VER_UNKNOWN;
    if (init == true) {
        return hardwareVersion;
    }
    uint32_t adcValue = GetHardwareVersionAdcValue();
    if (adcValue > 2000 && adcValue < 2100) {
        hardwareVersion = HW_VER_1_0;
    } else if (adcValue >= 2100 && adcValue < 2200) {
        hardwareVersion = HW_VER_1_1;
    } else if (adcValue >= 2200 && adcValue < 2300) {
        hardwareVersion = HW_VER_1_2;
    } else if (adcValue >= 2300 && adcValue < 2400) {
        hardwareVersion = HW_VER_1_3;
    }
    init = true;
    return hardwareVersion;
}

const char *GetHardwareVersionString(void)
{
    uint32_t version = GetHardwareVersion();
    if (version == HW_VER_1_0) {
        return "V 1.0";
    } else if (version == HW_VER_1_1) {
        return "V 1.1";
    } else if (version == HW_VER_1_2) {
        return "V 1.2";
    } else if (version == HW_VER_1_3) {
        return "V 1.3";
    } else {
        return "unknown";
    }
}
