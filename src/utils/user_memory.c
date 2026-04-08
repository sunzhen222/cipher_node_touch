#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_assert.h"
#include "user_memory.h"
#include "user_utils.h"
#include "FreeRTOSConfig.h"

#define SRAM_HEAP_TRACK                 0

static uint32_t g_sramHeapCount = 0;

void *SramMallocTrack(size_t size, const char *file, int line, const char *func)
{
    void *p = pvPortMalloc(size);
#if (SRAM_HEAP_TRACK == 1)
    printf("sram malloc:%s %s %d 0x%X %d\n", file, func, line, p, size);
#else
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
#endif
    ASSERT(p != NULL);
    g_sramHeapCount++;
    return p;
}

void SramFreeTrack(void *p, const char *file, int line, const char *func)
{
#if (SRAM_HEAP_TRACK == 1)
    printf("sram free:%s %s %d 0x%X\n", file, func, line, p);
#else
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
#endif
    if (p != NULL) {
        vPortFree(p);
        g_sramHeapCount--;
    }
}

void *SramReallocTrack(void *p, size_t size, const char *file, int line, const char *func)
{
    void *dest;
#if (SRAM_HEAP_TRACK == 1)
    printf("sram realloc:%s %s %d 0x%X %d\n", file, func, line, p, size);
#else
    UNUSED(file);
    UNUSED(line);
    UNUSED(func);
#endif
    dest = pvPortMalloc(size);
    ASSERT(dest != NULL);
    memcpy(dest, p, size);
    vPortFree(p);
    return dest;
}

void *SramMalloc(size_t size)
{
    void *p = pvPortMalloc((size_t) size);
    g_sramHeapCount++;
    return p;
}

void SramFree(void *p)
{
    if (p != NULL) {
        vPortFree(p);
        g_sramHeapCount--;
    }
}

void PrintHeapInfo(void)
{
    printf("sram heap info:\n");
    printf("g_sramHeapCount = %lu\n", g_sramHeapCount);
    printf("TotalHeapSize = %u\n", configTOTAL_HEAP_SIZE);                      // Total heap size
    printf("FreeHeapSize = %u\n", xPortGetFreeHeapSize());                      // Free heap space
    printf("MinEverFreeHeapSize = %u\n", xPortGetMinimumEverFreeHeapSize());    // Minimum amount of unallocated heap space
}
