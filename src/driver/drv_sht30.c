#include "drv_sht30.h"

#include "drv_i2c.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"

#define SHT30_I2C_ADDR_A                 0x88
#define SHT30_I2C_ADDR_B                 0x8A

#define SHT30_CMD_SINGLE_SHOT_HIGH       0x2400
#define SHT30_CMD_SOFT_RESET             0x30A2

#define SHT30_MEASURE_WAIT_MS            20
#define SHT30_I2C_TIMEOUT_MS             100

static uint8_t g_sht30Addr = SHT30_I2C_ADDR_A;
static bool g_sht30Ready;

static bool Sht30WriteCommand(uint16_t cmd);
static uint8_t Sht30CalcCrc8(const uint8_t *data, uint8_t len);

void Sht30Init(void)
{
    g_sht30Ready = false;

    if (HAL_I2C_IsDeviceReady(&hi2c2, SHT30_I2C_ADDR_A, 2, SHT30_I2C_TIMEOUT_MS) == HAL_OK) {
        g_sht30Addr = SHT30_I2C_ADDR_A;
    } else if (HAL_I2C_IsDeviceReady(&hi2c2, SHT30_I2C_ADDR_B, 2, SHT30_I2C_TIMEOUT_MS) == HAL_OK) {
        g_sht30Addr = SHT30_I2C_ADDR_B;
    } else {
        printf("SHT30 not found on I2C2\n");
        return;
    }

    if (!Sht30WriteCommand(SHT30_CMD_SOFT_RESET)) {
        printf("SHT30 soft reset failed\n");
        return;
    }

    HAL_Delay(2);
    g_sht30Ready = true;
    printf("SHT30 found on I2C2, addr=0x%02X\n", g_sht30Addr >> 1);
}

bool Sht30Read(Sht30Data_t *data)
{
    uint8_t cmd[2];
    uint8_t readBuf[6];
    uint16_t rawTemp;
    uint16_t rawRh;
    int32_t tempCx10;
    uint32_t rhx10;

    if (data == NULL || !g_sht30Ready) {
        return false;
    }

    cmd[0] = (uint8_t)(SHT30_CMD_SINGLE_SHOT_HIGH >> 8);
    cmd[1] = (uint8_t)(SHT30_CMD_SINGLE_SHOT_HIGH & 0xFF);
    if (HAL_I2C_Master_Transmit(&hi2c2, g_sht30Addr, cmd, sizeof(cmd), SHT30_I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }

    HAL_Delay(SHT30_MEASURE_WAIT_MS);
    if (HAL_I2C_Master_Receive(&hi2c2, g_sht30Addr, readBuf, sizeof(readBuf), SHT30_I2C_TIMEOUT_MS) != HAL_OK) {
        return false;
    }

    if (Sht30CalcCrc8(readBuf, 2) != readBuf[2] || Sht30CalcCrc8(readBuf + 3, 2) != readBuf[5]) {
        return false;
    }

    rawTemp = ((uint16_t)readBuf[0] << 8) | readBuf[1];
    rawRh = ((uint16_t)readBuf[3] << 8) | readBuf[4];

    tempCx10 = ((int32_t)1750 * (int32_t)rawTemp + 32767) / 65535 - 450;
    rhx10 = ((uint32_t)1000 * (uint32_t)rawRh + 32767) / 65535;
    if (rhx10 > 1000U) {
        rhx10 = 1000U;
    }

    data->temperatureCx10 = (int16_t)tempCx10;
    data->humidityRhx10 = (uint16_t)rhx10;
    return true;
}

static bool Sht30WriteCommand(uint16_t cmd)
{
    uint8_t buf[2];

    buf[0] = (uint8_t)(cmd >> 8);
    buf[1] = (uint8_t)(cmd & 0xFF);
    return HAL_I2C_Master_Transmit(&hi2c2, g_sht30Addr, buf, sizeof(buf), SHT30_I2C_TIMEOUT_MS) == HAL_OK;
}

static uint8_t Sht30CalcCrc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0xFF;
    uint8_t i;
    uint8_t bit;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (bit = 0; bit < 8; bit++) {
            if ((crc & 0x80U) != 0U) {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}
