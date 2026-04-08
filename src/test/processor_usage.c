#include "processor_usage.h"
#include "cmsis_os2.h"
#include "rtos_expand.h"
#include "user_utils.h"

static osTimerId_t g_processorUsageTimerId = NULL;

static void ProcessorUsageTimerCallback(void *argument)
{
    UNUSED(argument);
    PrintTasksFireWaterData();
}

void SetProcessorUsage(bool en)
{
    if (en) {
        if (g_processorUsageTimerId != NULL) {
            return;
        }
        g_processorUsageTimerId = osTimerNew(ProcessorUsageTimerCallback, osTimerPeriodic, NULL, NULL);
        osTimerStart(g_processorUsageTimerId, TASKS_FIRE_WATER_TICK);
    } else {
        if (g_processorUsageTimerId == NULL) {
            return;
        }
        osTimerStop(g_processorUsageTimerId);
        osTimerDelete(g_processorUsageTimerId);
        g_processorUsageTimerId = NULL;
    }
}


