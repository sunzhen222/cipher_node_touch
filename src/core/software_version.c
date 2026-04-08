#include "software_version.h"
#include "stdio.h"

#define SOFTWARE_VERSION_MAJOR      1
#define SOFTWARE_VERSION_MINOR      0
#define SOFTWARE_VERSION_BUILD      10

const char *GetSoftwareVersionString(void)
{
    static char version[32];
    sprintf(version, "V %d.%d.%d", SOFTWARE_VERSION_MAJOR, SOFTWARE_VERSION_MINOR, SOFTWARE_VERSION_BUILD);
    return version;
}

const char *GetBuildTime(void)
{
    return __DATE__ " " __TIME__;
}
