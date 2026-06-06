#include "drv_touch.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "drv_i2c.h"
#include "drv_gpio.h"
#include "user_utils.h"
#include "test_cmd.h"

#define CST726_I2C_ADDR                     0x34
#define FT6336_I2C_ADDR                     0x70

#define FT6336_REVERSE_X                    0
#define FT6336_REVERSE_Y                    0

#define TOUCH_RESET_GPIO                    GPIOC
#define TOUCH_RESET_PIN                     GPIO_PIN_3

#define TOUCH_RESET_HIGH()                  HAL_GPIO_WritePin(TOUCH_RESET_GPIO, TOUCH_RESET_PIN, GPIO_PIN_SET)
#define TOUCH_RESET_LOW()                   HAL_GPIO_WritePin(TOUCH_RESET_GPIO, TOUCH_RESET_PIN, GPIO_PIN_RESET)

#define TOUCH_INT_GPIO                      GPIOB
#define TOUCH_INT_PIN                       GPIO_PIN_8

typedef enum {
    TOUCH_PAD_CHIP_CST726 = 0,
    TOUCH_PAD_CHIP_FT6336,
    TOUCH_PAD_CHIP_UNKNOWN
} TouchPadChipType;

static int32_t Cst726GetStatus(TouchStatus_t *status);
static int32_t Ft6336GetStatus(TouchStatus_t *status);
static void TouchTestFunc(int argc, char *argv[]);

static TouchPadIntCallbackFunc_t g_touchPadIntCallback;
static TouchPadChipType g_touchPadChipType = TOUCH_PAD_CHIP_UNKNOWN;

void TouchInit(TouchPadIntCallbackFunc_t func)
{
    uint8_t addr = 0;
    TOUCH_RESET_HIGH();
    if (GetFirstI2cDeviceAddr(&hi2c1, &addr)) {
        if (addr == CST726_I2C_ADDR) {
            printf("touch pad chip: CST726\n");
            g_touchPadChipType = TOUCH_PAD_CHIP_CST726;
        } else if (addr == FT6336_I2C_ADDR) {
            printf("touch pad chip: FT6336\n");
            g_touchPadChipType = TOUCH_PAD_CHIP_FT6336;
        } else {
            printf("touch pad chip: unknown addr 0x%X\n", addr);
            g_touchPadChipType = TOUCH_PAD_CHIP_UNKNOWN;
        }
    } else {
        printf("no touch pad found\n");
        g_touchPadChipType = TOUCH_PAD_CHIP_UNKNOWN;
    }
    g_touchPadIntCallback = func;
    RegisterTestCmd("touch:", TouchTestFunc);

    //PB8, Touch int.
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = TOUCH_INT_PIN;
    gpioInit.Mode = GPIO_MODE_IT_FALLING;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(TOUCH_INT_GPIO, &gpioInit);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void TouchReset(void)
{
    TOUCH_RESET_LOW();
    osDelay(100);
    TOUCH_RESET_HIGH();
    osDelay(100);
}

void TouchOnOff(bool on)
{
    if (on) {
        TOUCH_RESET_HIGH();
    } else {
        TOUCH_RESET_LOW();
    }
}

/// @brief Get touch status, including touch state, X/Y coordinate.
/// @param status TouchStatus struct addr.
int32_t TouchGetStatus(TouchStatus_t *status)
{
    if (g_touchPadChipType == TOUCH_PAD_CHIP_CST726) {
        return Cst726GetStatus(status);
    } else if (g_touchPadChipType == TOUCH_PAD_CHIP_FT6336) {
        return Ft6336GetStatus(status);
    }
    return -1;
}

void TouchPadIntHandler(void)
{
    if (g_touchPadIntCallback != NULL) {
        g_touchPadIntCallback();
    }
}

static int32_t Cst726GetStatus(TouchStatus_t *status)
{
    HAL_StatusTypeDef ret;
    uint8_t readBuff[7] = {0};

    ret = HAL_I2C_Mem_Read(&hi2c1, CST726_I2C_ADDR, 0xD000, I2C_MEMADD_SIZE_16BIT, readBuff, 7, 100);
    if (ret != HAL_OK) {
        printf("i2c trans hall err:%d\n", ret);
        return -1;
    }
    if (readBuff[6] == 0xAB) {
        status->x = (unsigned int)((readBuff[2] << 4) | (readBuff[3] & 0x0F)) - 1;
        status->y = TOUCH_PAD_RES_Y - (unsigned int)((readBuff[1] << 4) | ((readBuff[3] >> 4) & 0x0F)) - 1;
        status->touch = (readBuff[0] & 0x0F) == 0x06 ? true : false;
        //printf("x=%hu,y=%hu,touch=%d\n", status->x, status->y, status->touch);
    } else {
        printf("touch data err\n");
        return -2;
    }
    return 0;
}

static int32_t Ft6336GetStatus(TouchStatus_t *status)
{
    HAL_StatusTypeDef ret;
    uint8_t readBuff[5] = {0};

    ret = HAL_I2C_Mem_Read(&hi2c1, FT6336_I2C_ADDR, 0x02, I2C_MEMADD_SIZE_8BIT, readBuff, 5, 100);
    if (ret != HAL_OK) {
        printf("i2c trans hall err:%d\n", ret);
        return -1;
    }
    //PrintArray("ft6336 rcv data", readBuff, 5);

    status->touch = readBuff[0] > 0 ? true : false;
    status->x = ((readBuff[1] & 0x0F) << 8) + readBuff[2];
#if (FT6336_REVERSE_X == 1)
    status->x = TOUCH_PAD_RES_X - status->x - 1;
#endif
    status->y = ((readBuff[3] & 0x0F) << 8) + readBuff[4];
#if (FT6336_REVERSE_Y == 1)
    status->y = TOUCH_PAD_RES_Y - status->y - 1;
#endif
    //printf("x=%hu,y=%hu,touch=%d\n", status->x, status->y, status->touch);

    return 0;
}

static void TouchTestFunc(int argc, char *argv[])
{
    if (argc == 0) {
        printf("touch test err\n");
        return;
    }
    if (strcmp(argv[0], "get") == 0) {
        TouchStatus_t status;
        printf("touch get\n");
        TouchGetStatus(&status);
    } else if (strcmp(argv[0], "scan") == 0) {
        I2cScan();
    }
}
