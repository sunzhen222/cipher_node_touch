/**************************************************************************************************
 * Copyright (c) Larrie studio. 2020-2025. All rights reserved.
 * Description: User file system interface.
 * Author: leon sun
 * Create: 2023-11-28
 ************************************************************************************************/

#ifndef _USER_FS_H
#define _USER_FS_H

#include "stdint.h"
#include "ff.h"

void FsMount(void);
int32_t FsFileLoad(const char *path, uint32_t offset, void *data, uint32_t size);
int32_t FsFileSave(const char *path, const void *data, uint32_t size);
int32_t FsFileAppend(const char *path, const void *data, uint32_t size);
int32_t FsFileMd5(uint8_t *result, const char *path);
int32_t FsFileSize(const char *path);
int32_t FindSuffixFiles(const char *path, const char *suffix, char fileList[][FF_LFN_BUF + 10], uint32_t *fileCount, uint32_t maxFiles);

void FatfsError(FRESULT errNum);
#endif
