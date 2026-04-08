#include "drv_i2c.h"
#include "stdio.h"
#include "user_assert.h"

static void I2c1Init(void);
//static void I2c3Init(void);

I2C_HandleTypeDef hi2c1;

/// @brief I2C hardware init
/// @param
void I2cInit(void)
{
    I2c1Init();
    I2cScan();
}

void I2cScan(void)
{
    uint16_t addr;
    uint8_t cmd = 0;
    printf("hi2c1 start scanning\n");
    for (addr = 0; addr < 255; addr++) {
        if (HAL_I2C_Master_Transmit(&hi2c1, addr, &cmd, 1, 10) == HAL_OK) {
            printf("hi2c1 0x%X has ack\n", addr);
        }
    }
    printf("hi2c1 scaning over\n");
    //printf("hi2c3 start scanning\n");
    //for (addr = 0; addr < 255; addr++) {
    //    if (HAL_I2C_Master_Transmit(&hi2c3, addr, &cmd, 1, 10) == HAL_OK) {
    //        printf("hi2c3 0x%X has ack\n", addr);
    //    }
    //}
    //printf("hi2c3 scaning over\n");
}

bool GetFirstI2cDeviceAddr(I2C_HandleTypeDef *hi2c, uint8_t *addr)
{
    uint8_t tempAddr;
    uint8_t cmd = 0;
    for (tempAddr = 0; tempAddr < 255; tempAddr++) {
        if (HAL_I2C_Master_Transmit(hi2c, tempAddr, &cmd, 1, 10) == HAL_OK) {
            *addr = tempAddr;
            return true;
        }
    }
    return false;
}

static void I2c1Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    hi2c1.Instance = I2C1;
    hi2c1.Init.ClockSpeed = 100000;
    hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    ASSERT(HAL_I2C_Init(&hi2c1) == HAL_OK);

    /**I2C1 GPIO Configuration
    PB6     ------> I2C1_SCL
    PB7     ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
