
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
#include "flash_map.h"
#include "user_fs.h"
#include "drv_i2c_io.h"
#include "drv_uart.h"
#include "drv_timer.h"
#include "processor_usage.h"
#include "battery.h"
#include "at_command.h"
#include "search_wifi.h"
#include "lvgl.h"

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
static void LvglMemInfoFunc(int argc, char *argv[]);
static void GetTickFunc(int argc, char *argv[]);
static void RebootFunc(int argc, char *argv[]);
static void EraseFlashFunc(int argc, char *argv[]);
static void WriteFlashFunc(int argc, char *argv[]);
static void ReadFlashFunc(int argc, char *argv[]);
static void FlashPerfFunc(int argc, char *argv[]);
static void HardfaultFunc(int argc, char *argv[]);
static void FileMd5Func(int argc, char *argv[]);
static void I2cFunc(int argc, char *argv[]);
static void ShowAssertFunc(int argc, char *argv[]);
static void TimerDmaFunc(int argc, char *argv[]);
static void FindFilesFunc(int argc, char *argv[]);
static void ProcessorUsageFunc(int argc, char *argv[]);
static void BatteryFunc(int argc, char *argv[]);
static void AtSendFunc(int argc, char *argv[]);
static void SearchWiFiFunc(int argc, char *argv[]);


static const TestCmdItem_t g_testCmdTable[] = {
    {"test",                    TestFunc                },
    {"all task info",           AllTaskInfoFunc         },
    {"clr cpu per",             ClrCpuPercentFunc       },
    {"heap info",               HeapInfoFunc            },
    {"lvgl mem",                LvglMemInfoFunc         },
    {"get tick",                GetTickFunc             },
    {"reboot",                  RebootFunc              },
    {"erase flash:",            EraseFlashFunc          },
    {"write flash:",            WriteFlashFunc          },
    {"read flash:",             ReadFlashFunc           },
    {"flash perf:",             FlashPerfFunc           },
    {"hardfault:",              HardfaultFunc           },
    {"file md5:",               FileMd5Func             },
    {"i2c:",                    I2cFunc                 },
    {"show assert",             ShowAssertFunc          },
    {"timer_dma:",              TimerDmaFunc            },
    {"find_files:",             FindFilesFunc           },
    {"processor_usage:",        ProcessorUsageFunc      },
    {"battery",                 BatteryFunc             },
    {"at:",                     AtSendFunc              },
    {"search wifi",             SearchWiFiFunc          },
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

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    lv_mem_monitor_t mon;
    size_t lv_used;
#endif

    printf("\n\n");
    printf("TotalHeapSize = %d\n", configTOTAL_HEAP_SIZE);
    printf("FreeHeapSize = %d\n", xPortGetFreeHeapSize());
    printf("MinEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    lv_mem_monitor(&mon);
    lv_used = mon.total_size - mon.free_size;
    printf("LvglTotalMemSize = %lu\n", (unsigned long)mon.total_size);
    printf("LvglFreeMemSize = %lu\n", (unsigned long)mon.free_size);
    printf("LvglUsedMemSize = %lu\n", (unsigned long)lv_used);
    printf("LvglMemFragPct = %u%%\n", mon.frag_pct);
#endif
}

static void LvglMemInfoFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    lv_mem_monitor_t mon;
    size_t used;

    lv_mem_monitor(&mon);
    used = mon.total_size - mon.free_size;

    printf("\nLVGL memory info:\n");
    printf("  total_size         = %lu\n", (unsigned long)mon.total_size);
    printf("  used_size          = %lu\n", (unsigned long)used);
    printf("  free_size          = %lu\n", (unsigned long)mon.free_size);
    printf("  free_biggest_size  = %lu\n", (unsigned long)mon.free_biggest_size);
    printf("  used_pct           = %u%%\n", mon.used_pct);
    printf("  frag_pct           = %u%%\n", mon.frag_pct);
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

static void PrintSpeed(uint32_t bytes, uint32_t us, const char *label)
{
    uint32_t kbpsX100;
    if (us == 0) {
        printf("%s=N/A", label);
        return;
    }
    kbpsX100 = (uint32_t)(((uint64_t)bytes * 100000000ULL) / ((uint64_t)1024 * us));
    printf("%s=%lu.%02luKB/s",
           label,
           kbpsX100 / 100U,
           kbpsX100 % 100U);
}

static void FlashPerfFunc(int argc, char *argv[])
{
    uint32_t startAddr;
    uint32_t size;
    uint32_t loopCount;
    uint32_t flashSize;
    uint32_t freeStart;
    uint32_t freeEnd;
    uint32_t alignedStart;
    uint32_t alignedEnd;
    uint32_t eraseUs;
    uint32_t writeUs;
    uint32_t readUs;
    uint32_t eraseUsTotal = 0;
    uint32_t writeUsTotal = 0;
    uint32_t readUsTotal = 0;
    uint8_t *writeBuf = SRAM_MALLOC(FATFS_FLASH_SECTOR_SIZE);
    uint8_t *readBuf = SRAM_MALLOC(FATFS_FLASH_SECTOR_SIZE);

    startAddr = SPI_FLASH_ADDR_ERR_INFO + SPI_FLASH_SIZE_ERR_INFO;
    size = FATFS_FLASH_SECTOR_SIZE * 16;
    loopCount = 1;

    if (argc >= 1) {
        sscanf(argv[0], "%lX", &startAddr);
    }
    if (argc >= 2) {
        sscanf(argv[1], "%lu", &size);
    }
    if (argc >= 3) {
        sscanf(argv[2], "%lu", &loopCount);
    }

    if (loopCount == 0) {
        printf("FLASH_PERF_ERROR invalid loopCount=0\n");
        SRAM_FREE(writeBuf);
        SRAM_FREE(readBuf);
        return;
    }
    if (size == 0) {
        printf("FLASH_PERF_ERROR invalid size=0\n");
        SRAM_FREE(writeBuf);
        SRAM_FREE(readBuf);
        return;
    }

    flashSize = w25qxx.SectorCount * w25qxx.SectorSize;
    freeStart = SPI_FLASH_ADDR_ERR_INFO + SPI_FLASH_SIZE_ERR_INFO;
    freeEnd = flashSize;
    if (freeStart >= freeEnd) {
        printf("FLASH_PERF_ERROR no_free_flash_region\n");
        SRAM_FREE(writeBuf);
        SRAM_FREE(readBuf);
        return;
    }

    alignedStart = startAddr & ~(FATFS_FLASH_SECTOR_SIZE - 1);
    alignedEnd = (startAddr + size + FATFS_FLASH_SECTOR_SIZE - 1) & ~(FATFS_FLASH_SECTOR_SIZE - 1);

    if (alignedStart < freeStart || alignedEnd > freeEnd || alignedStart >= alignedEnd) {
        printf("FLASH_PERF_ERROR out_of_range req=[0x%lX,0x%lX) free=[0x%lX,0x%lX)\n",
               alignedStart,
               alignedEnd,
               freeStart,
               freeEnd);
        SRAM_FREE(writeBuf);
        SRAM_FREE(readBuf);
        return;
    }

    size = alignedEnd - alignedStart;

    printf("FLASH_PERF_CONFIG addr=0x%lX size=%lu loop=%lu free_start=0x%lX free_end=0x%lX\n",
           alignedStart,
           size,
           loopCount,
           freeStart,
           freeEnd);

    for (uint32_t loop = 0; loop < loopCount; loop++) {
        uint32_t firstMismatchAddr = 0;
        uint32_t mismatchCount = 0;
        uint32_t t0;
        uint32_t t1;

        t0 = GetMicroSecCount();
        for (uint32_t addr = alignedStart; addr < alignedEnd; addr += FATFS_FLASH_SECTOR_SIZE) {
            W25qxx_EraseAddr(addr);
        }
        t1 = GetMicroSecCount();
        eraseUs = t1 - t0;

        t0 = GetMicroSecCount();
        for (uint32_t off = 0; off < size; off += FATFS_FLASH_SECTOR_SIZE) {
            uint32_t chunkSize = FATFS_FLASH_SECTOR_SIZE;
            if (off + chunkSize > size) {
                chunkSize = size - off;
            }
            for (uint32_t i = 0; i < chunkSize; i++) {
                writeBuf[i] = (uint8_t)((alignedStart + off + i) & 0xFF);
            }
            W25qxx_WriteBytes(writeBuf, alignedStart + off, chunkSize);
        }
        t1 = GetMicroSecCount();
        writeUs = t1 - t0;

        t0 = GetMicroSecCount();
        for (uint32_t off = 0; off < size; off += FATFS_FLASH_SECTOR_SIZE) {
            uint32_t chunkSize = FATFS_FLASH_SECTOR_SIZE;
            if (off + chunkSize > size) {
                chunkSize = size - off;
            }
            memset(readBuf, 0, chunkSize);
            W25qxx_ReadBytes(readBuf, alignedStart + off, chunkSize);
            for (uint32_t i = 0; i < chunkSize; i++) {
                uint8_t expected = (uint8_t)((alignedStart + off + i) & 0xFF);
                if (readBuf[i] != expected) {
                    if (mismatchCount == 0) {
                        firstMismatchAddr = alignedStart + off + i;
                    }
                    mismatchCount++;
                }
            }
        }
        t1 = GetMicroSecCount();
        readUs = t1 - t0;

        eraseUsTotal += eraseUs;
        writeUsTotal += writeUs;
        readUsTotal += readUs;

        printf("FLASH_PERF_LOOP idx=%lu erase_us=%lu ", loop + 1, eraseUs);
        PrintSpeed(size, eraseUs, "erase_speed");
        printf(" write_us=%lu ", writeUs);
        PrintSpeed(size, writeUs, "write_speed");
        printf(" read_us=%lu ", readUs);
        PrintSpeed(size, readUs, "read_speed");
        if (mismatchCount == 0) {
            printf(" verify=OK\n");
        } else {
            printf(" verify=FAIL mismatch=%lu first_addr=0x%lX\n", mismatchCount, firstMismatchAddr);
        }
    }

    printf("FLASH_PERF_SUMMARY loop=%lu size=%lu erase_avg_us=%lu write_avg_us=%lu read_avg_us=%lu\n",
           loopCount,
           size,
           eraseUsTotal / loopCount,
           writeUsTotal / loopCount,
           readUsTotal / loopCount);
    SRAM_FREE(writeBuf);
    SRAM_FREE(readBuf);
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

static void BatteryFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);
    PrintBatteryInfo();
}

static void AtSendFunc(int argc, char *argv[])
{
    if (argc > 0) {
        ClearReceivedAtCommand();
        SendAtCommand(argv[0]);
        char received[AT_COMMAND_MAX_LENGTH];
        while (GetReceivedAtCommand(received, 5000)) {
            printf("received:%s", received);
        }
    }
}

static const char *WiFiSecurityToString(WiFiSecurityType security)
{
    switch (security) {
    case WIFI_SECURITY_OPEN:
        return "OPEN";
    case WIFI_SECURITY_WEP:
        return "WEP";
    case WIFI_SECURITY_WPA:
        return "WPA";
    case WIFI_SECURITY_WPA2:
        return "WPA2";
    default:
        return "UNKNOWN";
    }
}

static void SearchWiFiFunc(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    WiFiItem_t wifiHead = {0};
    printf("searching wifi...\n");
    uint32_t count = SearchWiFi(&wifiHead);

    printf("wifi scan count=%lu\n", count);

    WiFiItem_t *node = wifiHead.next;
    uint32_t index = 1;
    while (node != NULL) {
        printf("%lu: ssid=%s ch=%u sec=%s rssi=%d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
               index,
               node->ssid,
               node->ch,
               WiFiSecurityToString(node->security),
               node->rssi,
               node->bssid[0],
               node->bssid[1],
               node->bssid[2],
               node->bssid[3],
               node->bssid[4],
               node->bssid[5]);
        node = node->next;
        index++;
    }

    FreeWiFiList(&wifiHead);
}
