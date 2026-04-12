
#ifndef _LORA_H
#define _LORA_H

#include "stdint.h"
#include "stdbool.h"
#include "llcc68/driver_llcc68.h"
#include "stm32f4xx_hal.h"

#define LLCC68_LORA_DEFAULT_STOP_TIMER_ON_PREAMBLE      LLCC68_BOOL_FALSE                   /**< disable stop timer on preamble */
#define LLCC68_LORA_DEFAULT_REGULATOR_MODE              LLCC68_REGULATOR_MODE_DC_DC_LDO     /**< only ldo */
#define LLCC68_LORA_DEFAULT_PA_CONFIG_DUTY_CYCLE        0x02                                /**< set +17dBm power */
#define LLCC68_LORA_DEFAULT_PA_CONFIG_HP_MAX            0x03                                /**< set +17dBm power */
#define LLCC68_LORA_DEFAULT_TX_DBM                      17                                  /**< +17dBm */
#define LLCC68_LORA_DEFAULT_RAMP_TIME                   LLCC68_RAMP_TIME_10US               /**< set ramp time 10 us */
#define LLCC68_LORA_DEFAULT_SF                          LLCC68_LORA_SF_7                    /**< sf7 */
#define LLCC68_LORA_DEFAULT_BANDWIDTH                   LLCC68_LORA_BANDWIDTH_250_KHZ       /**< 250khz */
#define LLCC68_LORA_DEFAULT_CR                          LLCC68_LORA_CR_4_5                  /**< cr4/5 */
#define LLCC68_LORA_DEFAULT_LOW_DATA_RATE_OPTIMIZE      LLCC68_BOOL_FALSE                   /**< disable low data rate optimize */
#define LLCC68_LORA_DEFAULT_RF_FREQUENCY                438000000U                          /**< 438000000Hz */
#define LLCC68_LORA_DEFAULT_SYMB_NUM_TIMEOUT            0                                   /**< 0 */
#define LLCC68_LORA_DEFAULT_SYNC_WORD                   0x3444U                             /**< public network */
#define LLCC68_LORA_DEFAULT_RX_GAIN                     0x94                                /**< common rx gain */
#define LLCC68_LORA_DEFAULT_OCP                         0x38                                /**< 140 mA */
#define LLCC68_LORA_DEFAULT_PREAMBLE_LENGTH             12                                  /**< 12 */
#define LLCC68_LORA_DEFAULT_HEADER                      LLCC68_LORA_HEADER_EXPLICIT         /**< explicit header */
#define LLCC68_LORA_DEFAULT_BUFFER_SIZE                 255                                 /**< 255 */
#define LLCC68_LORA_DEFAULT_CRC_TYPE                    LLCC68_LORA_CRC_TYPE_ON             /**< crc on */
#define LLCC68_LORA_DEFAULT_INVERT_IQ                   LLCC68_BOOL_FALSE                   /**< disable invert iq */
#define LLCC68_LORA_DEFAULT_CAD_SYMBOL_NUM              LLCC68_LORA_CAD_SYMBOL_NUM_2        /**< 2 symbol */
#define LLCC68_LORA_DEFAULT_CAD_DET_PEAK                24                                  /**< 24 */
#define LLCC68_LORA_DEFAULT_CAD_DET_MIN                 10                                  /**< 10 */
#define LLCC68_LORA_DEFAULT_START_MODE                  LLCC68_START_MODE_WARM              /**< warm mode */
#define LLCC68_LORA_DEFAULT_RTC_WAKE_UP                 LLCC68_BOOL_TRUE                    /**< enable rtc wake up */

#define LORA_TX_LED_ON()            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
#define LORA_TX_LED_OFF()           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)

#define LORA_RX_LED_ON()            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET)
#define LORA_RX_LED_OFF()           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET)


void LoraInit(void);
uint8_t LoraSettings(void);
const char *LoraGetWirelessStatus(void);
void LoraSendData(const void *data, uint16_t length);
void DoLoraSendData(const void *data, uint16_t length);
uint8_t llcc68_lora_set_send_mode(void);
uint8_t llcc68_lora_send(const uint8_t *buf, uint16_t len);
uint8_t llcc68_lora_set_continuous_receive_mode(void);
uint8_t llcc68_lora_get_status(float *rssi, float *snr);
uint8_t llcc68_lora_check_packet_error(llcc68_bool_t *enable);
void LoraIrqHandler(void);
bool LoraTxBusy(void);

#endif
