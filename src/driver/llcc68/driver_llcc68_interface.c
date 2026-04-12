/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * @file      driver_llcc68_interface_template.c
 * @brief     driver llcc68 interface template source file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2023-04-15
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2023/04/15  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#include "driver_llcc68_interface.h"
#include "drv_spi.h"
#include "drv_gpio.h"
#include "user_utils.h"
#include "user_delay.h"

#define LLCC68_RESET_GPIO                   GPIOC
#define LLCC68_RESET_PIN                    GPIO_PIN_7

#define LLCC68_BUSY_GPIO                    GPIOC
#define LLCC68_BUSY_PIN                     GPIO_PIN_6

#define LLCC68_CS_GPIO                      GPIOB
#define LLCC68_CS_PIN                       GPIO_PIN_12

/**
 * @brief  interface spi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi init failed
 * @note   none
 */
uint8_t llcc68_interface_spi_init(void)
{
    SetGpioOutput(LLCC68_CS_GPIO, LLCC68_CS_PIN, GPIO_PIN_SET);
    return 0;
}

/**
 * @brief  interface spi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi deinit failed
 * @note   none
 */
uint8_t llcc68_interface_spi_deinit(void)
{
    return 0;
}

/**
 * @brief      interface spi bus write read
 * @param[in]  *in_buf pointer to a input buffer
 * @param[in]  in_len input length
 * @param[out] *out_buf pointer to a output buffer
 * @param[in]  out_len output length
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t llcc68_interface_spi_write_read(uint8_t *in_buf, uint32_t in_len,
                                        uint8_t *out_buf, uint32_t out_len)
{
    __disable_irq();
    HAL_GPIO_WritePin(LLCC68_CS_GPIO, LLCC68_CS_PIN, GPIO_PIN_RESET);
    if (in_buf != NULL && in_len > 0) {
        if (HAL_SPI_Transmit(&hspi2, in_buf, in_len, 100) != HAL_OK) {
            HAL_GPIO_WritePin(LLCC68_CS_GPIO, LLCC68_CS_PIN, GPIO_PIN_SET);
            __enable_irq();
            return 1;
        }
    }
    if (out_buf != NULL && out_len > 0) {
        if (HAL_SPI_Receive(&hspi2, out_buf, out_len, 100) != HAL_OK) {
            HAL_GPIO_WritePin(LLCC68_CS_GPIO, LLCC68_CS_PIN, GPIO_PIN_SET);
            __enable_irq();
            return 1;
        }
    }
    HAL_GPIO_WritePin(LLCC68_CS_GPIO, LLCC68_CS_PIN, GPIO_PIN_SET);
    __enable_irq();
    return 0;
}

/**
 * @brief  interface reset gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_init(void)
{
    SetGpioOutput(LLCC68_RESET_GPIO, LLCC68_RESET_PIN, GPIO_PIN_SET);
    return 0;
}

/**
 * @brief  interface reset gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_reset_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief     interface reset gpio write
 * @param[in] data written data
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t llcc68_interface_reset_gpio_write(uint8_t data)
{
    if (data != 0) {
        HAL_GPIO_WritePin(LLCC68_RESET_GPIO, LLCC68_RESET_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LLCC68_RESET_GPIO, LLCC68_RESET_PIN, GPIO_PIN_RESET);
    }
    return 0;
}

/**
 * @brief  interface busy gpio init
 * @return status code
 *         - 0 success
 *         - 1 init failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_init(void)
{
    SetGpioInput(LLCC68_BUSY_GPIO, LLCC68_BUSY_PIN, GPIO_NOPULL);
    return 0;
}

/**
 * @brief  interface busy gpio deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t llcc68_interface_busy_gpio_deinit(void)
{
    return 0;
}

/**
 * @brief      interface busy gpio read
 * @param[out] *value pointer to a value buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t llcc68_interface_busy_gpio_read(uint8_t *value)
{
    *value = HAL_GPIO_ReadPin(LLCC68_BUSY_GPIO, LLCC68_BUSY_PIN);
    return 0;
}

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void llcc68_interface_delay_ms(uint32_t ms)
{
    UserDelay(ms);
}

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void llcc68_interface_debug_print(const char *const fmt, ...)
{
    printf(fmt);
}

/**
 * @brief     interface receive callback
 * @param[in] type receive callback type
 * @param[in] *buf pointer to a buffer address
 * @param[in] len buffer length
 * @note      none
 */
void llcc68_interface_receive_callback(uint16_t type, uint8_t *buf, uint16_t len)
{
    UNUSED(type);
    UNUSED(buf);
    UNUSED(len);
    switch (type) {
    case LLCC68_IRQ_TX_DONE : {
        llcc68_interface_debug_print("llcc68: irq tx done.\n");

        break;
    }
    case LLCC68_IRQ_RX_DONE : {
        llcc68_interface_debug_print("llcc68: irq rx done.\n");

        break;
    }
    case LLCC68_IRQ_PREAMBLE_DETECTED : {
        llcc68_interface_debug_print("llcc68: irq preamble detected.\n");

        break;
    }
    case LLCC68_IRQ_SYNC_WORD_VALID : {
        llcc68_interface_debug_print("llcc68: irq valid sync word detected.\n");

        break;
    }
    case LLCC68_IRQ_HEADER_VALID : {
        llcc68_interface_debug_print("llcc68: irq valid header.\n");

        break;
    }
    case LLCC68_IRQ_HEADER_ERR : {
        llcc68_interface_debug_print("llcc68: irq header error.\n");

        break;
    }
    case LLCC68_IRQ_CRC_ERR : {
        llcc68_interface_debug_print("llcc68: irq crc error.\n");

        break;
    }
    case LLCC68_IRQ_CAD_DONE : {
        llcc68_interface_debug_print("llcc68: irq cad done.\n");

        break;
    }
    case LLCC68_IRQ_CAD_DETECTED : {
        llcc68_interface_debug_print("llcc68: irq cad detected.\n");

        break;
    }
    case LLCC68_IRQ_TIMEOUT : {
        llcc68_interface_debug_print("llcc68: irq timeout.\n");

        break;
    }
    default : {
        llcc68_interface_debug_print("llcc68: unknown code.\n");

        break;
    }
    }
}
