#include "snapshot.h"

#include "ff.h"
#include "lvgl.h"
#include "drv_button.h"
#include "lv_port_disp.h"
#include "user_memory.h"
#include "stdio.h"
#include "string.h"

#define SNAPSHOT_BMP_PATH         "0:snapshot.bmp"
#define BMP_FILE_HEADER_SIZE      14U
#define BMP_INFO_HEADER_SIZE      40U
#define BMP_HEADER_SIZE           (BMP_FILE_HEADER_SIZE + BMP_INFO_HEADER_SIZE)

typedef struct {
    bool active;
    int32_t error;
    FIL fp;
    lv_color_format_t colorFormat;
    uint32_t width;
    uint32_t height;
    uint32_t rowStride;
    uint32_t pixelDataOffset;
    uint8_t *lineBuf;
} SnapshotCaptureCtx_t;

static SnapshotCaptureCtx_t *g_captureCtx = NULL;

static void WriteLe16(uint8_t *buf, uint16_t value)
{
    buf[0] = (uint8_t)(value & 0xFFU);
    buf[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

static void WriteLe32(uint8_t *buf, uint32_t value)
{
    buf[0] = (uint8_t)(value & 0xFFU);
    buf[1] = (uint8_t)((value >> 8U) & 0xFFU);
    buf[2] = (uint8_t)((value >> 16U) & 0xFFU);
    buf[3] = (uint8_t)((value >> 24U) & 0xFFU);
}

static FRESULT WriteAll(FIL *fp, const void *data, UINT size)
{
    UINT writeBytes = 0;
    FRESULT res = f_write(fp, data, size, &writeBytes);
    if (res != FR_OK) {
        return res;
    }

    if (writeBytes != size) {
        return FR_DISK_ERR;
    }

    return FR_OK;
}

static void SnapshotCaptureStop(void)
{
    lv_port_disp_set_flush_observer(NULL);

    if (g_captureCtx == NULL) {
        return;
    }

    if (g_captureCtx->active) {
        f_close(&g_captureCtx->fp);
    }

    if (g_captureCtx->lineBuf != NULL) {
        SRAM_FREE(g_captureCtx->lineBuf);
        g_captureCtx->lineBuf = NULL;
    }

    g_captureCtx->active = false;
    g_captureCtx = NULL;
}

static uint8_t PixelSize(lv_color_format_t cf)
{
    switch (cf) {
    case LV_COLOR_FORMAT_RGB565:
        return 2;
    case LV_COLOR_FORMAT_RGB888:
        return 3;
    case LV_COLOR_FORMAT_XRGB8888:
    case LV_COLOR_FORMAT_ARGB8888:
        return 4;
    default:
        return 0;
    }
}

static void ConvertPixelToBgr24(const uint8_t *src, lv_color_format_t cf, uint8_t *dst)
{
    switch (cf) {
    case LV_COLOR_FORMAT_RGB565: {
        uint16_t v = (uint16_t)src[0] | ((uint16_t)src[1] << 8U);
        uint8_t r5 = (uint8_t)((v >> 11U) & 0x1FU);
        uint8_t g6 = (uint8_t)((v >> 5U) & 0x3FU);
        uint8_t b5 = (uint8_t)(v & 0x1FU);

        dst[0] = (uint8_t)((b5 * 255U) / 31U);
        dst[1] = (uint8_t)((g6 * 255U) / 63U);
        dst[2] = (uint8_t)((r5 * 255U) / 31U);
        break;
    }
    case LV_COLOR_FORMAT_RGB888:
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        break;
    case LV_COLOR_FORMAT_XRGB8888:
    case LV_COLOR_FORMAT_ARGB8888:
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        break;
    default:
        dst[0] = 0;
        dst[1] = 0;
        dst[2] = 0;
        break;
    }
}

static void SnapshotFlushObserver(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LV_UNUSED(disp);

    if (g_captureCtx == NULL || !g_captureCtx->active || g_captureCtx->error < 0) {
        return;
    }

    uint32_t areaWidth = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t areaHeight = (uint32_t)(area->y2 - area->y1 + 1);
    uint8_t pixelSize = PixelSize(g_captureCtx->colorFormat);
    if (pixelSize == 0U) {
        g_captureCtx->error = -7;
        return;
    }

    if (g_captureCtx->lineBuf == NULL) {
        g_captureCtx->error = -8;
        return;
    }

    for (uint32_t y = 0; y < areaHeight; y++) {
        const uint8_t *srcRow = px_map + y * areaWidth * pixelSize;

        for (uint32_t x = 0; x < areaWidth; x++) {
            ConvertPixelToBgr24(srcRow + x * pixelSize, g_captureCtx->colorFormat, g_captureCtx->lineBuf + x * 3U);
        }

        uint32_t absoluteY = (uint32_t)area->y1 + y;
        uint32_t bmpY = g_captureCtx->height - 1U - absoluteY;
        uint32_t writeOffset = g_captureCtx->pixelDataOffset + bmpY * g_captureCtx->rowStride + (uint32_t)area->x1 * 3U;

        FRESULT res = f_lseek(&g_captureCtx->fp, writeOffset);
        if (res != FR_OK) {
            g_captureCtx->error = -(int32_t)res;
            return;
        }

        res = WriteAll(&g_captureCtx->fp, g_captureCtx->lineBuf, (UINT)(areaWidth * 3U));
        if (res != FR_OK) {
            g_captureCtx->error = -(int32_t)res;
            return;
        }
    }
}

int32_t SnapshotSaveBmp(void)
{
    SnapshotCaptureCtx_t *capture = SRAM_MALLOC(sizeof(SnapshotCaptureCtx_t));
    lv_display_t *disp = lv_display_get_default();
    lv_obj_t *screen = lv_screen_active();
    FRESULT res;

    if (capture == NULL) {
        return -10;
    }

    memset(capture, 0, sizeof(SnapshotCaptureCtx_t));

    if (disp == NULL || screen == NULL) {
        return -1;
    }

    capture->colorFormat = lv_display_get_color_format(disp);
    capture->width = (uint32_t)lv_display_get_horizontal_resolution(disp);
    capture->height = (uint32_t)lv_display_get_vertical_resolution(disp);
    capture->rowStride = (capture->width * 3U + 3U) & ~3U;
    capture->pixelDataOffset = BMP_HEADER_SIZE;
    capture->error = 0;
    capture->active = false;
    capture->lineBuf = SRAM_MALLOC(capture->width * 3U);
    g_captureCtx = capture;

    if (capture->lineBuf == NULL) {
        SRAM_FREE(capture);
        return -9;
    }

    if (PixelSize(capture->colorFormat) == 0U) {
        SnapshotCaptureStop();
        SRAM_FREE(capture);
        return -7;
    }

    uint32_t rowBytes = capture->width * 3U;
    uint32_t pixelDataSize = capture->rowStride * capture->height;
    uint32_t bmpFileSize = BMP_HEADER_SIZE + pixelDataSize;

    uint8_t bmpHeader[BMP_HEADER_SIZE] = {0};

    bmpHeader[0] = 'B';
    bmpHeader[1] = 'M';
    WriteLe32(&bmpHeader[2], bmpFileSize);
    WriteLe32(&bmpHeader[10], BMP_HEADER_SIZE);
    WriteLe32(&bmpHeader[14], BMP_INFO_HEADER_SIZE);
    WriteLe32(&bmpHeader[18], capture->width);
    WriteLe32(&bmpHeader[22], capture->height);
    WriteLe16(&bmpHeader[26], 1U);
    WriteLe16(&bmpHeader[28], 24U);
    WriteLe32(&bmpHeader[34], pixelDataSize);

    res = f_open(&capture->fp, SNAPSHOT_BMP_PATH, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        SnapshotCaptureStop();
        SRAM_FREE(capture);
        return -res;
    }

    capture->active = true;

    res = WriteAll(&capture->fp, bmpHeader, BMP_HEADER_SIZE);
    if (res != FR_OK) {
        SnapshotCaptureStop();
        SRAM_FREE(capture);
        return -res;
    }

    if (capture->rowStride > rowBytes) {
        uint8_t pad[3] = {0};
        uint32_t padding = capture->rowStride - rowBytes;
        for (uint32_t i = 0; i < capture->height; i++) {
            res = WriteAll(&capture->fp, pad, (UINT)padding);
            if (res != FR_OK) {
                SnapshotCaptureStop();
                SRAM_FREE(capture);
                return -res;
            }
        }
        res = f_lseek(&capture->fp, BMP_HEADER_SIZE);
        if (res != FR_OK) {
            SnapshotCaptureStop();
            SRAM_FREE(capture);
            return -res;
        }
    }

    lv_port_disp_set_flush_observer(SnapshotFlushObserver);

    lv_obj_invalidate(screen);
    lv_refr_now(disp);

    int32_t captureError = capture->error;
    SnapshotCaptureStop();
    SRAM_FREE(capture);

    if (captureError < 0) {
        return captureError;
    }

    return (int32_t)bmpFileSize;
}

static void SnapshotFunc(void)
{
    int32_t result = SnapshotSaveBmp();
    if (result >= 0) {
        printf("Snapshot saved to " SNAPSHOT_BMP_PATH ", size=%ld bytes\n", result);
    } else {
        printf("Snapshot failed, error code=%ld\n", result);
    }
}

void RegisterButtonSnapshot(void)
{
    RegisterButtonEvent(BUTTON_EVENT_LONG_PRESS, SnapshotFunc);
}

