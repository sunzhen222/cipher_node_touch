#include "lora.h"
#include "llcc68/driver_llcc68.h"
#include "llcc68/driver_llcc68_interface.h"
#include "user_utils.h"
#include "stm32f4xx_hal.h"
#include "background_task.h"
#include "user_msg.h"
#include "drv_gpio.h"
#include "device_settings.h"
#include "test_cmd.h"
#include "protocol_parse.h"
#include <stdlib.h>

#define LORA_SEND_TICK_ENABLE    0

static void LoraCallback(uint16_t type, uint8_t *buf, uint16_t len);
static void LoraTestFunc(int argc, char *argv[]);

llcc68_handle_t g_llcc64Handle, g_llcc64Handle2;
static volatile bool g_loraTxBusy = false;
typedef enum {
    LORA_WIRELESS_STATUS_UNKNOWN = 0,
    LORA_WIRELESS_STATUS_INIT_OK,
    LORA_WIRELESS_STATUS_INIT_FAILED,
} LoraWirelessStatus_t;
static volatile LoraWirelessStatus_t g_loraWirelessStatus = LORA_WIRELESS_STATUS_UNKNOWN;
#if (LORA_SEND_TICK_ENABLE == 1)
static uint32_t g_sendTick = 0;
#endif

const char *LoraGetWirelessStatus(void)
{
    switch (g_loraWirelessStatus) {
    case LORA_WIRELESS_STATUS_INIT_OK:
        return "ok";
    case LORA_WIRELESS_STATUS_INIT_FAILED:
        return "error";
    default:
        return "Unknown";
    }
}

void LoraInit(void)
{
    uint8_t ret;
    uint8_t modulation;
    uint8_t config;

    g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;

    DRIVER_LLCC68_LINK_INIT(&g_llcc64Handle, llcc68_handle_t);
    DRIVER_LLCC68_LINK_SPI_INIT(&g_llcc64Handle, llcc68_interface_spi_init);
    DRIVER_LLCC68_LINK_SPI_DEINIT(&g_llcc64Handle, llcc68_interface_spi_deinit);
    DRIVER_LLCC68_LINK_SPI_WRITE_READ(&g_llcc64Handle, llcc68_interface_spi_write_read);
    DRIVER_LLCC68_LINK_RESET_GPIO_INIT(&g_llcc64Handle, llcc68_interface_reset_gpio_init);
    DRIVER_LLCC68_LINK_RESET_GPIO_DEINIT(&g_llcc64Handle, llcc68_interface_reset_gpio_deinit);
    DRIVER_LLCC68_LINK_RESET_GPIO_WRITE(&g_llcc64Handle, llcc68_interface_reset_gpio_write);
    DRIVER_LLCC68_LINK_BUSY_GPIO_INIT(&g_llcc64Handle, llcc68_interface_busy_gpio_init);
    DRIVER_LLCC68_LINK_BUSY_GPIO_DEINIT(&g_llcc64Handle, llcc68_interface_busy_gpio_deinit);
    DRIVER_LLCC68_LINK_BUSY_GPIO_READ(&g_llcc64Handle, llcc68_interface_busy_gpio_read);
    DRIVER_LLCC68_LINK_DELAY_MS(&g_llcc64Handle, llcc68_interface_delay_ms);
    DRIVER_LLCC68_LINK_DEBUG_PRINT(&g_llcc64Handle, llcc68_interface_debug_print);
    DRIVER_LLCC68_LINK_RECEIVE_CALLBACK(&g_llcc64Handle, LoraCallback);

    do {
        /* init the llcc68 */
        ret = llcc68_init(&g_llcc64Handle);
        CHECK_ERRCODE_BREAK("llcc68_init", ret);
        /* enter standby */
        ret = llcc68_set_standby(&g_llcc64Handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
        CHECK_ERRCODE_BREAK("llcc68_set_standby", ret);
        /* set stop timer on preamble */
        ret = llcc68_set_stop_timer_on_preamble(&g_llcc64Handle, LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE);
        CHECK_ERRCODE_BREAK("llcc68_set_stop_timer_on_preamble", ret);
        /* set regulator mode */
        ret = llcc68_set_regulator_mode(&g_llcc64Handle, LLCC68_LORA_DEFAULT_REGULATOR_MODE);
        CHECK_ERRCODE_BREAK("llcc68_set_regulator_mode", ret);
        /* set pa config */
        ret = llcc68_set_pa_config(&g_llcc64Handle, LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE, LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX);
        CHECK_ERRCODE_BREAK("llcc68_set_pa_config", ret);
        /* enter to stdby xosc mode */
        ret = llcc68_set_rx_tx_fallback_mode(&g_llcc64Handle, LLCC68_RX_TX_FALLBACK_MODE_STDBY_XOSC);
        CHECK_ERRCODE_BREAK("llcc68_set_rx_tx_fallback_mode", ret);
        /* set dio irq */
        ret = llcc68_set_dio_irq_params(&g_llcc64Handle, 0x03FF, 0x03FF, 0x0000, 0x0000);
        CHECK_ERRCODE_BREAK("llcc68_set_dio_irq_params", ret);
        /* clear irq status */
        ret = llcc68_clear_irq_status(&g_llcc64Handle, 0x03FF);
        CHECK_ERRCODE_BREAK("llcc68_clear_irq_status", ret);
        /* set lora mode */
        ret = llcc68_set_packet_type(&g_llcc64Handle, LLCC68_PACKET_TYPE_LORA);
        CHECK_ERRCODE_BREAK("llcc68_set_packet_type", ret);
        /* set tx params */
        ret = llcc68_set_tx_params(&g_llcc64Handle, LLCC68_LORA_DEFAULT_TX_DBM, LLCC68_LORA_DEFAULT_RAMP_TIME);
        CHECK_ERRCODE_BREAK("llcc68_set_tx_params", ret);
        /* set base address */
        ret = llcc68_set_buffer_base_address(&g_llcc64Handle, 0x00, 0x00);
        CHECK_ERRCODE_BREAK("llcc68_set_buffer_base_address", ret);
        /* set lora symb num */
        ret = llcc68_set_lora_symb_num_timeout(&g_llcc64Handle, LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT);
        CHECK_ERRCODE_BREAK("llcc68_set_lora_symb_num_timeout", ret);
        /* reset stats */
        ret = llcc68_reset_stats(&g_llcc64Handle, 0x0000, 0x0000, 0x0000);
        CHECK_ERRCODE_BREAK("llcc68_reset_stats", ret);
        /* clear device errors */
        ret = llcc68_clear_device_errors(&g_llcc64Handle);
        CHECK_ERRCODE_BREAK("llcc68_clear_device_errors", ret);
        /* set the lora sync word */
        ret = llcc68_set_lora_sync_word(&g_llcc64Handle, LLCC68_LORA_DEFAULT_SYNC_WORD);
        CHECK_ERRCODE_BREAK("llcc68_set_lora_sync_word", ret);
        /* get tx modulation */
        ret = llcc68_get_tx_modulation(&g_llcc64Handle, (uint8_t *)&modulation);
        CHECK_ERRCODE_BREAK("llcc68_get_tx_modulation", ret);
        modulation |= 0x04;
        /* set the tx modulation */
        ret = llcc68_set_tx_modulation(&g_llcc64Handle, modulation);
        CHECK_ERRCODE_BREAK("llcc68_set_tx_modulation", ret);
        /* set the rx gain */
        ret = llcc68_set_rx_gain(&g_llcc64Handle, LLCC68_LORA_DEFAULT_RX_GAIN);
        CHECK_ERRCODE_BREAK("llcc68_set_rx_gain", ret);
        /* set the ocp */
        ret = llcc68_set_ocp(&g_llcc64Handle, LLCC68_LORA_DEFAULT_OCP);
        CHECK_ERRCODE_BREAK("llcc68_set_ocp", ret);
        /* get the tx clamp config */
        ret = llcc68_get_tx_clamp_config(&g_llcc64Handle, (uint8_t *)&config);
        CHECK_ERRCODE_BREAK("llcc68_get_tx_clamp_config", ret);
        config |= 0x1E;
        /* set the tx clamp config */
        ret = llcc68_set_tx_clamp_config(&g_llcc64Handle, config);
        CHECK_ERRCODE_BREAK("llcc68_set_tx_clamp_config", ret);
    } while (0);
    if (ret == 0) {
        ret = LoraSettings();
        if (ret != 0) {
            printf("LoraSettings err,0x%X\r\n", ret);
        }
    }

    RegisterTestCmd("lora:", LoraTestFunc);
}

uint8_t LoraSettings(void)
{
    uint8_t ret;
    uint32_t reg;

    ret = llcc68_set_standby(&g_llcc64Handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ);
    if (ret != 0) {
        g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;
        return 1;
    }

    ASSERT(DeviceSettingsGetLoraSpreadingFactor() >= LLCC68_LORA_SF_5 && DeviceSettingsGetLoraSpreadingFactor() <= LLCC68_LORA_SF_11);
    ASSERT(DeviceSettingsGetLoraBandwidth() >= LLCC68_LORA_BANDWIDTH_125_KHZ && DeviceSettingsGetLoraBandwidth() <= LLCC68_LORA_BANDWIDTH_500_KHZ);
    ret = llcc68_set_lora_modulation_params(&g_llcc64Handle, DeviceSettingsGetLoraSpreadingFactor(), DeviceSettingsGetLoraBandwidth(),
                                            LLCC68_LORA_DEFAULT_CR, LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE);
    if (ret != 0) {
        g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;
        return 1;
    }

    ASSERT(DeviceSettingsGetLoraFreq() >= 410000000 && DeviceSettingsGetLoraFreq() <= 525000000);
    ret = llcc68_frequency_convert_to_register(&g_llcc64Handle, DeviceSettingsGetLoraFreq(), (uint32_t *)&reg);
    if (ret != 0) {
        g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;
        return 1;
    }
    ret = llcc68_set_rf_frequency(&g_llcc64Handle, reg);
    if (ret != 0) {
        g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;
        return 1;
    }

    ret = llcc68_lora_set_continuous_receive_mode();
    if (ret != 0) {
        g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_FAILED;
        return 1;
    }

    g_loraWirelessStatus = LORA_WIRELESS_STATUS_INIT_OK;
    return 0;
}

void LoraSendData(const void *data, uint16_t length)
{
    //PrintArray("send lora", data, length);
    PubBufferMsg(WIRELESS_MSG_LORA_SEND, data, length);
}

void DoLoraSendData(const void *data, uint16_t length)
{
    uint8_t ret;
    do {
#if (LORA_SEND_TICK_ENABLE == 1)
        g_sendTick = HAL_GetTick();
#endif
        LORA_TX_LED_ON();
        ret = llcc68_lora_set_send_mode();
        CHECK_ERRCODE_BREAK("llcc68_lora_set_send_mode", ret);
        ret = llcc68_lora_send(data, length);
        CHECK_ERRCODE_BREAK("llcc68_lora_send", ret);
    } while (0);
    //PrintArray("send lora", data, length);
}

/**
 * @brief  lora example enter to the send mode
 * @return status code
 *         - 0 success
 *         - 1 enter failed
 * @note   none
 */
uint8_t llcc68_lora_set_send_mode(void)
{
    /* set dio irq */
    if (llcc68_set_dio_irq_params(&g_llcc64Handle, LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_TX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0) {
        return 1;
    }

    /* clear irq status */
    if (llcc68_clear_irq_status(&g_llcc64Handle, 0x03FFU) != 0) {
        return 1;
    }

    return 0;
}

/**
 * @brief     lora example send lora data
 * @param[in] *buf points to a data buffer
 * @param[in] len is the data length
 * @return    status code
 *            - 0 success
 *            - 1 send failed
 * @note      none
 */
uint8_t llcc68_lora_send(const uint8_t *buf, uint16_t len)
{
    /* send the data */
    g_loraTxBusy = true;
    if (llcc68_lora_transmit(&g_llcc64Handle, LLCC68_CLOCK_SOURCE_XTAL_32MHZ,
                             LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH, LLCC68_LORA_DEFAULT_HEADER,
                             LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ,
                             (uint8_t *)buf, len, 0) != 0) {
        return 1;
    }

    return 0;
}

/**
 * @brief  lora example enter to the continuous receive mode
 * @return status code
 *         - 0 success
 *         - 1 enter failed
 * @note   none
 */
uint8_t llcc68_lora_set_continuous_receive_mode(void)
{
    uint8_t setup;

    /* set dio irq */
    if (llcc68_set_dio_irq_params(&g_llcc64Handle, LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  LLCC68_IRQ_RX_DONE | LLCC68_IRQ_TIMEOUT | LLCC68_IRQ_CRC_ERR | LLCC68_IRQ_CAD_DONE | LLCC68_IRQ_CAD_DETECTED,
                                  0x0000, 0x0000) != 0) {
        return 1;
    }

    /* clear irq status */
    if (llcc68_clear_irq_status(&g_llcc64Handle, 0x03FFU) != 0) {
        return 1;
    }

    /* set lora packet params */
    if (llcc68_set_lora_packet_params(&g_llcc64Handle, LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH,
                                      LLCC68_LORA_DEFAULT_HEADER, LLCC68_LORA_DEFAULT_BUFFER_SIZE,
                                      LLCC68_LORA_DEFAULT_CRC_TYPE, LLCC68_LORA_DEFAULT_INVERT_IQ) != 0) {
        return 1;
    }

    /* get iq polarity */
    if (llcc68_get_iq_polarity(&g_llcc64Handle, (uint8_t *)&setup) != 0) {
        return 1;
    }

#if LLCC68_LORA_DEFAULT_INVERT_IQ == LLCC68_BOOL_FALSE
    setup |= 1 << 2;
#else
    setup &= ~(1 << 2);
#endif

    /* set the iq polarity */
    if (llcc68_set_iq_polarity(&g_llcc64Handle, setup) != 0) {
        return 1;
    }

    /* start receive */
    if (llcc68_continuous_receive(&g_llcc64Handle) != 0) {
        return 1;
    }

    return 0;
}

/**
 * @brief      lora example get the status
 * @param[out] *rssi points to a rssi buffer
 * @param[out] *snr points to a snr buffer
 * @return     status code
 *             - 0 success
 *             - 1 get status failed
 * @note       none
 */
uint8_t llcc68_lora_get_status(float *rssi, float *snr)
{
    uint8_t rssi_pkt_raw;
    int8_t snr_pkt_raw;
    uint8_t signal_rssi_pkt_raw;
    float signal_rssi_pkt;

    /* get the status */
    if (llcc68_get_lora_packet_status(&g_llcc64Handle, (uint8_t *)&rssi_pkt_raw, (int8_t *)&snr_pkt_raw,
                                      (uint8_t *)&signal_rssi_pkt_raw, (float *)rssi, (float *)snr, (float *)&signal_rssi_pkt) != 0) {
        return 1;
    }

    return 0;
}

/**
 * @brief      lora example check packet error
 * @param[out] *enable points to a bool value buffer
 * @return     status code
 *             - 0 success
 *             - 1 check packet error failed
 * @note       none
 */
uint8_t llcc68_lora_check_packet_error(llcc68_bool_t *enable)
{
    /* check the error */
    if (llcc68_check_packet_error(&g_llcc64Handle, enable) != 0) {
        return 1;
    }

    return 0;
}
void LoraIrqHandler(void)
{
    llcc68_irq_handler(&g_llcc64Handle);
}

bool LoraTxBusy(void)
{
    return g_loraTxBusy;
}

static void LoraCallback(uint16_t type, uint8_t *buf, uint16_t len)
{
    switch (type) {
    case LLCC68_IRQ_TX_DONE : {
        g_loraTxBusy = false;
        llcc68_lora_set_continuous_receive_mode();
        LORA_TX_LED_OFF();
#if (LORA_SEND_TICK_ENABLE == 1)
        printf("send tick=%lu\n", HAL_GetTick() - g_sendTick);
#endif
    }
    break;
    case LLCC68_IRQ_RX_DONE : {
        llcc68_bool_t enable;

        /* check the error */
        if (llcc68_lora_check_packet_error(&enable) != 0) {
            return;
        }
        if ((enable == LLCC68_BOOL_FALSE) && len) {
            LORA_RX_LED_ON();
            ProtocolReceivedData(buf, len);
            PrintArray("received data", buf, len);
            LORA_RX_LED_OFF();
            float rssi, snr;
            if (llcc68_lora_get_status(&rssi, &snr) == 0) {
                printf("rssi:%.2f, snr:%.2f\n", rssi, snr);
            } else {
                printf("lora get status err\n");
            }
        } else {
            printf("lora rx error:%d\n", enable);
        }
    }
    break;
    case LLCC68_IRQ_PREAMBLE_DETECTED : {
        printf("llcc68: irq preamble detected.\n");
    }
    break;
    case LLCC68_IRQ_SYNC_WORD_VALID : {
        printf("llcc68: irq valid sync word detected.\n");
    }
    break;
    case LLCC68_IRQ_HEADER_VALID : {
        printf("llcc68: irq valid header.\n");
    }
    break;
    case LLCC68_IRQ_HEADER_ERR : {
        printf("llcc68: irq header error.\n");
    }
    break;
    case LLCC68_IRQ_CRC_ERR : {
        printf("llcc68: irq crc error.\n");
    }
    break;
    case LLCC68_IRQ_CAD_DONE : {
        printf("llcc68: irq cad done.\n");
    }
    break;
    case LLCC68_IRQ_CAD_DETECTED : {
        printf("llcc68: irq cad detected.\n");
    }
    break;
    case LLCC68_IRQ_TIMEOUT : {
        printf("llcc68: irq timeout.\n");
    }
    break;
    default:
        break;
    }
}

static void LoraTestFunc(int argc, char *argv[])
{
    unsigned long reg;
    char *end;
    uint8_t value;

    if (argc == 0) {
        printf("lora test err\n");
        return;
    }
    if (strcmp(argv[0], "send") == 0) {
        if (argc == 1) {
            printf("lora send err\n");
            return;
        }
        printf("send data:%s\n", argv[1]);
        LoraSendData((uint8_t *)argv[1], strlen(argv[1]));
    } else if (strcmp(argv[0], "readreg") == 0) {
        if (argc == 1) {
            printf("usage: lora readreg <reg>\n");
            return;
        }
        reg = strtoul(argv[1], &end, 0);
        if ((end == argv[1]) || (*end != '\0')) {
            printf("lora readreg invalid reg:%s\n", argv[1]);
            return;
        }
        if (llcc68_read_register(&g_llcc64Handle, (uint16_t)reg, &value, 1) != 0) {
            printf("lora read reg 0x%04lX failed\n", reg);
            return;
        }
        printf("lora reg[0x%04lX] = 0x%02X\n", reg, value);
    } else {
        printf("lora test err\n");
    }
}
