
#include "user_fs.h"
#include "stdio.h"
#include "md5.h"
#include "string.h"
#include "user_memory.h"

#define FS_MKFS_WORKBUF_SIZE 4096U

void FsMount(void)
{
    FRESULT res;
    static FATFS g_spiFlashFs;
    uint8_t *mkfsWorkBuf = NULL;
    MKFS_PARM mkfsOpt = {0};

    res = f_mount(&g_spiFlashFs, "0:", 1);
    if (res == FR_NO_FILESYSTEM) {
        printf("f_mount no filesystem, auto format...\n");
        mkfsWorkBuf = SRAM_MALLOC(FS_MKFS_WORKBUF_SIZE);

        // Use SFD (no MBR partition table) so the full SPI flash range is usable.
        mkfsOpt.fmt = FM_ANY | FM_SFD;
        mkfsOpt.n_fat = 0;
        mkfsOpt.align = 0;
        mkfsOpt.n_root = 0;
        mkfsOpt.au_size = 0;

        res = f_mkfs("0:", &mkfsOpt, mkfsWorkBuf, FS_MKFS_WORKBUF_SIZE);
        if (res != FR_OK) {
            printf("f_mkfs err=%d\n", res);
            SRAM_FREE(mkfsWorkBuf);
            return;
        }
        SRAM_FREE(mkfsWorkBuf);

        res = f_mount(&g_spiFlashFs, "0:", 1);
        if (res != FR_OK) {
            printf("f_mount after mkfs err=%d\n", res);
            return;
        }
        printf("filesystem auto formatted and mounted\n");
        return;
    }

    if (res != FR_OK) {
        printf("f_mount err=%d\n", res);
    }
}


int32_t FsFileLoad(const char *path, uint32_t offset, void *data, uint32_t size)
{
    FIL file;
    UINT readBytes = 0;
    FRESULT res;

    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        return -res;
    }

    if (offset >= f_size(&file)) {
        f_close(&file);
        return -res;
    }

    res = f_lseek(&file, offset);
    if (res != FR_OK) {
        f_close(&file);
        return -res;
    }

    res = f_read(&file, data, size, &readBytes);
    if (res != FR_OK) {
        return -res;
    }

    f_close(&file);

    return (int32_t)readBytes;
}


int32_t FsFileSave(const char *path, const void *data, uint32_t size)
{
    FIL file;
    UINT writtenBytes = 0;
    FRESULT res;

    res = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        return -res;
    }

    res = f_write(&file, data, size, &writtenBytes);
    if (res != FR_OK) {
        f_close(&file);
        return -res;
    }

    f_close(&file);

    return (int32_t)writtenBytes;
}


int32_t FsFileAppend(const char *path, const void *data, uint32_t size)
{
    FIL file;
    UINT writtenBytes = 0;
    FRESULT res;

    res = f_open(&file, path, FA_OPEN_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        return -res;
    }

    res = f_lseek(&file, f_size(&file));
    if (res != FR_OK) {
        f_close(&file);
        return -res;
    }

    res = f_write(&file, data, size, &writtenBytes);
    if (res != FR_OK) {
        f_close(&file);
        return -res;
    }

    f_close(&file);

    return (int32_t)writtenBytes;
}


int32_t FsFileMd5(uint8_t *result, const char *path)
{
    FIL fp;
    MD5_CTX ctx;
    uint32_t fileSize = 5000;
    uint32_t readBytes = 0;
    int len, changePercent = 0, percent;
    uint8_t fileBuf[256];
    FRESULT res;
    res = f_open(&fp, path, FA_OPEN_EXISTING | FA_READ);
    if (res) {
        FatfsError(res);
        return -res;
    }
    fileSize = f_size(&fp);
    int lastLen = fileSize;
    len = lastLen > 256 ? 256 : lastLen;
    printf("file size=%ld, please wait.\n", fileSize);
    MD5_Init(&ctx);
    while (lastLen) {
        len = lastLen > 256 ? 256  : lastLen;
        res = f_read(&fp, (void*)fileBuf, len, (UINT *)&readBytes);
        if (res) {
            FatfsError(res);
            f_close(&fp);
            return -res;
        }
        lastLen -= len;
        MD5_Update(&ctx, fileBuf, len);
        percent = (fileSize - lastLen) * 100 / fileSize;
        //printf("lastLen=%d,fileSize=%d\n", lastLen, fileSize);
        if (percent != changePercent) {
            changePercent = percent;
            printf("md5 calc percent = %ld\n", (fileSize - lastLen) * 100 / fileSize);
        }
    }
    MD5_Final(result, &ctx);
    f_close(&fp);

    return 0;
}

int32_t FsFileSize(const char *path)
{
    FIL file;
    FRESULT res;

    res = f_open(&file, path, FA_READ);
    if (res == FR_OK) {
        FSIZE_t size = f_size(&file);
        f_close(&file);
        return size;
    } else {
        return -res;
    }
}

static int HasSuffix(const char *name, const char *suffix)
{
    size_t nameLen = strlen(name);
    size_t suffixLen = strlen(suffix);
    if (nameLen < suffixLen) {
        return 0;
    }
    return (strcmp(name + nameLen - suffixLen, suffix) == 0);
}

int32_t FindSuffixFiles(const char *path, const char *suffix, char fileList[][FF_LFN_BUF + 10], uint32_t *fileCount, uint32_t maxFiles)
{
    DIR dir;
    FILINFO fno;
    FRESULT res;
    uint32_t count = 0;

    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        return -res;
    }

    while (1) {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0) {
            break;  // Error or end of directory
        }
        if (fno.fattrib & AM_DIR) {
            continue;  // Skip directories
        }
        if (HasSuffix(fno.fname, suffix)) {
            if (count < maxFiles) {
                if (strlen(path) == 2 && path[1] == ':') {
                    // Root directory case
                    snprintf(fileList[count], FF_LFN_BUF + 10, "%s%s", path, fno.fname);
                } else {
                    snprintf(fileList[count], FF_LFN_BUF + 10, "%s/%s", path, fno.fname);
                }
                count++;
            } else {
                break;  // Reached max files
            }
        }
    }

    f_closedir(&dir);
    *fileCount = count;
    return 0;
}

void FatfsError(FRESULT errNum)
{
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0" "INVALID_PARAMETER\0";
    FRESULT i;

    for (i = FR_OK; i != errNum && *str; i++) {
        while (*str++) ;
    }
    printf("errNum = %u FR_%s\n", (UINT)errNum, str);
}


