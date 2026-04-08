
#include "test_cmd.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_assert.h"
#include "task.h"
#include "user_memory.h"
#include "user_utils.h"
#include "rtos_expand.h"
#include "drv_w25qxx.h"
#include "user_fs.h"
#include "drv_i2c_io.h"
#include "drv_uart.h"
#include "drv_timer.h"
#include "processor_usage.h"

#define CMD_MAX_ARGC        16

typedef struct {
    const char *cmdString;
    TestCmdFunc_t func;
} TestCmdItem_t;

typedef struct __TestCmdNode_t {
    TestCmdItem_t testItem;
    struct __TestCmdNode_t *next;
} TestCmdNode_t;

static bool CompareToTestItem(const char *inputString, const TestCmdItem_t *item);

static void TestFunc(int argc, char *argv[]);
static void AllTaskInfoFunc(int argc, char *argv[]);
static void ClrCpuPercentFunc(int argc, char *argv[]);
static void HeapInfoFunc(int argc, char *argv[]);
static void GetTickFunc(int argc, char *argv[]);
static void RebootFunc(int argc, char *argv[]);
static void EraseFlashFunc(int argc, char *argv[]);
static void WriteFlashFunc(int argc, char *argv[]);
static void ReadFlashFunc(int argc, char *argv[]);
static void HardfaultFunc(int argc, char *argv[]);
static void FileMd5Func(int argc, char *argv[]);
static void I2cFunc(int argc, char *argv[]);
static void ShowAssertFunc(int argc, char *argv[]);
static void SendUart2Func(int argc, char *argv[]);
static void TimerDmaFunc(int argc, char *argv[]);
static void FindFilesFunc(int argc, char *argv[]);
static void ProcessorUsageFunc(int argc, char *argv[]);


static const TestCmdItem_t g_testCmdTable[] = {
    {"test",                    TestFunc                },
    {"all task info",           AllTaskInfoFunc         },
    {"clr cpu per",             ClrCpuPercentFunc       },
    {"heap info",               HeapInfoFunc            },
    {"get tick",                GetTickFunc             },
    {"reboot",                  RebootFunc              },
    {"erase flash:",            EraseFlashFunc          },
    {"write flash:",            WriteFlashFunc          },
    {"read flash:",             ReadFlashFunc           },
    {"hardfault:",              HardfaultFunc           },
    {"file md5:",               FileMd5Func             },
    {"i2c:",                    I2cFunc                 },
    {"show assert",             ShowAssertFunc          },
    {"send_uart2:",             SendUart2Func           },
    {"timer_dma:",              TimerDmaFunc            },
    {"find_files:",             FindFilesFunc           },
    {"processor_usage:",        ProcessorUsageFunc      },
};

TestCmdNode_t g_testCmdListHead = {0};

void RegisterTestCmd(const char *cmdString, const TestCmdFunc_t func)
{
    TestCmdNode_t *node, *lastNode;

    node = &g_testCmdListHead;
    if (node->testItem.cmdString == NULL) {
        node->testItem.cmdString = cmdString;
        node->testItem.func = func;
        node->next = NULL;
        return;
    }
    lastNode = node;
    node = node->next;
    while (node != NULL) {
        lastNode = node;
        node = node->next;
    }
    node = SRAM_MALLOC(sizeof(TestCmdNode_t));
    lastNode->next = node;
    memset(node, 0, sizeof(TestCmdNode_t));
    node->testItem.cmdString = cmdString;
    node->testItem.func = func;
    node->next = NULL;
}

bool CompareAndRunTestCmd(const char *inputString)
{
    uint32_t tableSize;
    TestCmdNode_t *node;

    tableSize = sizeof(g_testCmdTable) / sizeof(g_testCmdTable[0]);
    for (uint32_t i = 0; i < tableSize; i++) {
        if (CompareToTestItem(inputString, &g_testCmdTable[i])) {
            return true;
        }
    }
    node = &g_testCmdListHead;
    while (1) {
        if (CompareToTestItem(inputString, &node->testItem)) {
            return true;
        }
        if (node->next == NULL) {
            break;
        }
        node = node->next;
    }

    return false;
}

static bool CompareToTestItem(const char *inputString, const TestCmdItem_t *item)
{
    uint32_t compareLen, argc, argvLen;
    char *inputHead, *argvHead, *argv[CMD_MAX_ARGC];
    bool content = false;

    inputHead = strstr(item->cmdString, ":");
    if (inputHead != NULL) {
        // partial compare
        inputHead++;
        compareLen = inputHead - item->cmdString;
        if (strncmp(inputString, item->cmdString, compareLen) == 0) {
            inputHead = (char *)inputString + compareLen;
            argc = 0;
            for (uint32_t j = 0;; j++) {
                if (content) {
                    // content mode, finding space.
                    if (inputHead[j] == ' ' || inputHead[j] == 0) {
                        argvLen = &inputHead[j] - argvHead + 1; // including zero tail.
                        argv[argc] = SRAM_MALLOC(argvLen);
                        strncpy(argv[argc], argvHead, argvLen - 1);
                        argv[argc][argvLen - 1] = 0;
                        argc++;
                        content = false;
                    }
                } else {
                    // space mode, finding content.
                    if (inputHead[j] != ' ') {
                        argvHead = &inputHead[j];
                        content = true;
                    }
                }
                if (inputHead[j] == 0) {
                    break;
                }
            }
            item->func(argc, argv);
            for (uint32_t j = 0; j < argc; j++) {
                SRAM_FREE(argv[j]);
            }
            return true;
        }
    } else {
        // full compare
        if (strcmp(inputString, item->cmdString) == 0) {
            item->func(0, NULL);
            return true;
        }
    }
    return false;
}

static void TestFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    printf("test!!\n");
}

static void AllTaskInfoFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    PrintTasksStatus();
}

static void ClrCpuPercentFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    printf("clear cpu percent to 0\n");
    ClrRunTimeStats();
}

static void HeapInfoFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    printf("\n\n");
    printf("TotalHeapSize = %d\n", configTOTAL_HEAP_SIZE);
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
    printf("MinEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());
}

static void GetTickFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    uint32_t osTick = osKernelGetTickCount();
    uint32_t remainingSec = osTick / 1000;
    uint32_t day, hour, min, sec;
    day = remainingSec / 86400;
    remainingSec %= 86400;
    hour = remainingSec / 3600;
    remainingSec %= 3600;
    min = remainingSec / 60;
    remainingSec %= 60;
    sec = remainingSec;
    printf("os tick=%lu\n", osTick);
    printf("sys start for %ludays %luhours %luminute %lusecond\n", day, hour, min, sec);
    printf("micro second:%lu\n", GetMicroSecCount());
}

static void RebootFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    NVIC_SystemReset();
}

static void EraseFlashFunc(int argc, char *argv[])
{
    uint32_t addr;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%lX", &addr);
    printf("addr=0x%lX\n", addr);
    W25qxx_EraseAddr((uint32_t)addr);
    printf("erase over\n");
}

static void WriteFlashFunc(int argc, char *argv[])
{
    uint32_t addr, length;
    uint8_t *data;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%lX", &addr);
    sscanf(argv[1], "%lu", &length);
    printf("addr=0x%lX,length=%lu\n", addr, length);
    data = SRAM_MALLOC(length);
    for (uint32_t i = 0; i < length; i++) {
        data[i] = i;
    }
    W25qxx_WriteBytes(data, (uint32_t)addr, length);
    SRAM_FREE(data);
    printf("write over\n");
}

static void ReadFlashFunc(int argc, char *argv[])
{
    uint32_t addr, length;
    uint8_t *data;
    VALUE_CHECK(argc, 2);
    sscanf(argv[0], "%lX", &addr);
    sscanf(argv[1], "%lu", &length);
    printf("addr=0x%lX,length=%lu\n", addr, length);
    data = SRAM_MALLOC(length);
    memset(data, 0, length);
    W25qxx_ReadBytes(data, (uint32_t)addr, length);
    PrintArray("read", data, length);
    SRAM_FREE(data);
}

static void HardfaultFunc(int argc, char *argv[])
{
    uint32_t type;
    uint32_t *errAddr;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%lu", &type);
    printf("going to hardfault,type=%lu\r\n", type);
    switch (type) {
    case 0: {
        errAddr = (uint32_t *)0x00000000;
        *errAddr = 0x12345678;
        printf("*errAddr=0x%08lX\r\n", *errAddr);
    }
    break;
    default:
        break;
    }
}

static void FileMd5Func(int argc, char *argv[])
{
    UNUSED(argc);
    uint8_t md5Result[16];
    FsFileMd5(md5Result, argv[0]);
    PrintArray("md5", md5Result, 16);
}

static void I2cFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    I2CIO_Cfg_t i2cioConfig;
    I2CIO_Init(&i2cioConfig, GPIOA, GPIO_PIN_8, GPIOC, GPIO_PIN_9);
    I2CIO_SearchDevices(&i2cioConfig);
}

static void ShowAssertFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    ASSERT(0);
}

static void SendUart2Func(int argc, char *argv[])
{
    uint32_t length;
    VALUE_CHECK(argc, 1);
    uint8_t *hex = StrToHex(argv[0]);
    if (hex == NULL) {
        printf("invalid hex string\n");
        return;
    }
    length = strlen(argv[0]) / 2;
    SendUart2Data(hex, length);
    SRAM_FREE(hex);
}

static void TimerDmaFunc(int argc, char *argv[])
{
    uint32_t length;
    VALUE_CHECK(argc, 1);
    uint8_t *hex = StrToHex(argv[0]);
    if (hex == NULL) {
        printf("invalid hex string\n");
        return;
    }
    length = strlen(argv[0]) / 4;
    Timer1DmaStart((uint16_t *)hex, length);
    PrintU16Array("timer dma data", (uint16_t *)hex, length);
    //SRAM_FREE(hex);
}

static void FindFilesFunc(int argc, char *argv[])
{
    char fileList[10][FF_LFN_BUF + 10];
    uint32_t fileCount = 0;
    VALUE_CHECK(argc, 1);
    FindSuffixFiles("0:", argv[0], fileList, &fileCount, 10);
    printf("found %lu files with suffix %s\n", fileCount, argv[0]);
    for (uint32_t i = 0; i < fileCount; i++) {
        printf("  %s\n", fileList[i]);
    }
}

static void ProcessorUsageFunc(int argc, char *argv[])
{
    uint32_t en = 0;
    VALUE_CHECK(argc, 1);
    sscanf(argv[0], "%lu", &en);
    printf("processor usage %s\n", en ? "enabled" : "disabled");
    SetProcessorUsage(en != 0);
}
