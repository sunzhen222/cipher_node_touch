#include "drv_lcd.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "user_utils.h"
#include "test_cmd.h"
#include "drv_gpio.h"
#include "drv_spi.h"
#include "cmsis_os2.h"
#include "user_memory.h"
#include "lvgl.h"
#include "drv_timer.h"
#include "hardware_version.h"

#define LCD_RESET_GPIO                  GPIOC
#define LCD_RESET_PIN                   GPIO_PIN_4

#define LCD_DC_GPIO                     GPIOC
#define LCD_DC_PIN                      GPIO_PIN_5

#define LCD_CS_GPIO                     GPIOB
#define LCD_CS_PIN                      GPIO_PIN_0

#define LCD_RESET_HIGH()                HAL_GPIO_WritePin(LCD_RESET_GPIO, LCD_RESET_PIN, GPIO_PIN_SET)
#define LCD_RESET_LOW()                 HAL_GPIO_WritePin(LCD_RESET_GPIO, LCD_RESET_PIN, GPIO_PIN_RESET)

#define LCD_DC_HIGH()                   HAL_GPIO_WritePin(LCD_DC_GPIO, LCD_DC_PIN, GPIO_PIN_SET)
#define LCD_DC_LOW()                    HAL_GPIO_WritePin(LCD_DC_GPIO, LCD_DC_PIN, GPIO_PIN_RESET)

#define LCD_CS_HIGH()                   HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_SET)
#define LCD_CS_LOW()                    HAL_GPIO_WritePin(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_RESET)

SPI_HandleTypeDef *g_lcdSpi = NULL;

static void LcdTestFunc(int argc, char *argv[]);
static void LcdSpiWrite(const void *buff, uint32_t num);
static void LcdSpiTransmitBlocking(const uint8_t *buff, uint16_t len);
static void LcdWriteCmd(uint8_t cmd);
static void LcdWritByte(uint8_t byte);
static void LcdSetAddress(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);
static void LcdConfig(void);

void LcdInit(void)
{
    g_lcdSpi = &hspi1;
    SetGpioOutput(LCD_RESET_GPIO, LCD_RESET_PIN, GPIO_PIN_SET);
    SetGpioOutput(LCD_DC_GPIO, LCD_DC_PIN, GPIO_PIN_SET);
    SetGpioOutput(LCD_CS_GPIO, LCD_CS_PIN, GPIO_PIN_SET);
    LcdReset();
    LcdConfig();
    RegisterTestCmd("lcd:", LcdTestFunc);
    LcdClear(0);
    Timer5Init();
    SetLcdBackLight(100);
}

void SetLcdBackLight(uint32_t brightness)
{
    uint16_t duty;
    if (brightness > 100) {
        brightness = 100;
    }
    duty = brightness * 10;
    SetTimer5Duty(duty);
}

void LcdReset(void)
{
    LCD_RESET_LOW();
    osDelay(100);
    LCD_RESET_HIGH();
    osDelay(200);
}

void LcdDraw(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY, const void *map)
{
    uint32_t bytes;
    bytes = (endY - startY + 1) * (endX - startX + 1) * 3;
    LcdSetAddress(startX, startY, endX, endY);
    LCD_DC_HIGH();
    LcdSpiWrite(map, bytes);
}

void LcdClear(uint32_t color)
{
    uint32_t line = 10;
    uint8_t *mem = SRAM_MALLOC(320 * line * 3);
    for (uint32_t i = 0; i < 320 * line; i++) {
        mem[i * 3] = (color >> 16) & 0xFF;
        mem[i * 3 + 1] = (color >> 8) & 0xFF;
        mem[i * 3 + 2] = color & 0xFF;
    }

    LcdSetAddress(0, 0, LCD_DISPLAY_WIDTH - 1, LCD_DISPLAY_HEIGHT - 1);
    LCD_DC_HIGH();
    for (uint32_t i = 0; i < LCD_DISPLAY_HEIGHT / line; i++) {
        LcdSpiWrite(mem, LCD_DISPLAY_WIDTH * line * 3);
    }
    SRAM_FREE(mem);
}

static void LcdSpiWrite(const void *buff, uint32_t num)
{
    const uint8_t *ptr = (const uint8_t *)buff;
    uint32_t remaining = num;

    LCD_CS_LOW();
    while (remaining > 0) {
        uint16_t chunk = (remaining > 0xFFFFU) ? 0xFFFFU : (uint16_t)remaining;
        LcdSpiTransmitBlocking(ptr, chunk);
        ptr += chunk;
        remaining -= chunk;
    }
    LCD_CS_HIGH();
}

static void LcdSpiTransmitBlocking(const uint8_t *buff, uint16_t len)
{
    if (HAL_SPI_Transmit_DMA(g_lcdSpi, (uint8_t *)buff, len) != HAL_OK) {
        printf("lcd spi transmit dma error\r\n");
        HAL_SPI_Transmit(g_lcdSpi, (uint8_t *)buff, len, 100);
        return;
    }

    while (HAL_SPI_GetState(g_lcdSpi) != HAL_SPI_STATE_READY) {
    }
}

static void LcdWriteCmd(uint8_t cmd)
{
    LCD_DC_LOW();
    LcdSpiWrite(&cmd, 1);
}

static void LcdWritByte(uint8_t byte)
{
    LCD_DC_HIGH();
    LcdSpiWrite(&byte, 1);
}

static void LcdSetAddress(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    LcdWriteCmd(0x2A);
    LcdWritByte(startX >> 8);
    LcdWritByte(startX);
    LcdWritByte(endX >> 8);
    LcdWritByte(endX);

    LcdWriteCmd(0x2B);
    LcdWritByte(startY >> 8);
    LcdWritByte(startY);
    LcdWritByte(endY >> 8);
    LcdWritByte(endY);

    LcdWriteCmd(0x2C);
}

static void LcdConfig(void)
{
    osDelay(120);
    LcdWriteCmd(0xF7);
    LcdWritByte(0xA9);
    LcdWritByte(0x51);
    LcdWritByte(0x2C);
    LcdWritByte(0x82);
    LcdWriteCmd(0x36);
    LcdWritByte(0x40);      //RGB-BGR Order
    LcdWriteCmd(0x3A);
    LcdWritByte(0x55);
    LcdWriteCmd(0xB4);
    LcdWritByte(0x02);
    LcdWriteCmd(0xB1);
    LcdWritByte(0xA0);
    LcdWritByte(0x11);
    LcdWriteCmd(0xC0);
    LcdWritByte(0x0F);
    LcdWritByte(0x0F);
    LcdWriteCmd(0xc1);
    LcdWritByte(0x41);
    LcdWriteCmd(0xC2);
    LcdWritByte(0x22);
    LcdWriteCmd(0xB7);
    LcdWritByte(0xC6);
    LcdWriteCmd(0xc5);
    LcdWritByte(0x00);
    LcdWritByte(0x53);
    LcdWritByte(0x80);
    LcdWriteCmd(0xE0);
    LcdWritByte(0x00);
    LcdWritByte(0x08);
    LcdWritByte(0x0c);
    LcdWritByte(0x02);
    LcdWritByte(0x0e);
    LcdWritByte(0x04);
    LcdWritByte(0x30);
    LcdWritByte(0x45);
    LcdWritByte(0x47);
    LcdWritByte(0x04);
    LcdWritByte(0x0C);
    LcdWritByte(0x0a);
    LcdWritByte(0x2e);
    LcdWritByte(0x34);
    LcdWritByte(0x0F);
    LcdWriteCmd(0xE1);
    LcdWritByte(0x00);
    LcdWritByte(0x11);
    LcdWritByte(0x0d);
    LcdWritByte(0x01);
    LcdWritByte(0x0f);
    LcdWritByte(0x05);
    LcdWritByte(0x39);
    LcdWritByte(0x36);
    LcdWritByte(0x51);
    LcdWritByte(0x06);
    LcdWritByte(0x0f);
    LcdWritByte(0x0d);
    LcdWritByte(0x33);
    LcdWritByte(0x37);
    LcdWritByte(0x0F);
    LcdWriteCmd(0x21);
    LcdWritByte(0x00);
    LcdWriteCmd(0x3A);
    LcdWritByte(0x66);      //RGB666

    LcdWriteCmd(0x11);
    osDelay(120);
    LcdWriteCmd(0x29);
    osDelay(50);
}

static void LcdTestFunc(int argc, char *argv[])
{
    if (strcmp(argv[0], "bl") == 0) {
        uint32_t brightness;
        VALUE_CHECK(argc, 2);
        sscanf(argv[1], "%lu", &brightness);
        SetLcdBackLight(brightness);
    } else if (strcmp(argv[0], "draw") == 0) {
        uint16_t x1, y1, x2, y2;
        uint16_t *colorArray, color;
        uint32_t pixel;
        VALUE_CHECK(argc, 6);
        sscanf(argv[1], "%hu", &x1);
        sscanf(argv[2], "%hu", &y1);
        sscanf(argv[3], "%hu", &x2);
        sscanf(argv[4], "%hu", &y2);
        sscanf(argv[5], "%hX", &color);
        pixel = (y2 - y1 + 1) * (x2 - x1 + 1);
        colorArray = SRAM_MALLOC(pixel * 2);
        for (uint32_t i = 0; i < pixel; i++) {
            colorArray[i] = color;
        }
        LcdDraw(x1, y1, x2, y2, colorArray);
        SRAM_FREE(colorArray);
    } else if (strcmp(argv[0], "clear") == 0) {
        uint16_t colors[] = {0xF800, 0x07E0, 0x001F, 0xFFE0, 0xF81F, 0x07FF, 0xFFFF, 0x0000, 0xFD20, 0x8410};
        uint32_t colorCount = sizeof(colors) / sizeof(colors[0]);
        uint32_t startTick, endTick, totalTime;
        float avgFps;

        startTick = osKernelGetTickCount();
        for (uint32_t i = 0; i < colorCount; i++) {
            LcdClear(colors[i]);
        }
        endTick = osKernelGetTickCount();

        totalTime = endTick - startTick;
        avgFps = (float)colorCount * 1000.0f / (float)totalTime;
        printf("clear %lu times, total time: %lu ms, avg fps: %.2f\n", colorCount, totalTime, avgFps);
    } else {
        printf("lcd test input err\n");
    }
}

