#include "drv_uart.h"
#include "string.h"
#include "cmd_task.h"
#include "cm_backtrace.h"
#include "user_delay.h"
#include "user_utils.h"
#include "drv_timer.h"
#include "at_command.h"

#define UART2_RX_LEN        256
#define UART2_RX_IDLE_MS    10

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

static uint8_t g_uart1RxByte, g_uart2RxByte;


void Uart1Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 921600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    ASSERT(HAL_UART_Init(&huart1) == HAL_OK);

    /**UART1 GPIO Configuration
    PA9         ------> USART1_TX
    PA10        ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void Uart1Start(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *) &g_uart1RxByte, 1);
}

void Uart2Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    ASSERT(HAL_UART_Init(&huart2) == HAL_OK);

    /**UART2 GPIO Configuration
    PA2         ------> USART2_TX
    PA3         ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART2_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

    //Timer4Init();
}

void Uart2Start(void)
{
    HAL_UART_Receive_IT(&huart2, (uint8_t *) &g_uart2RxByte, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        HAL_UART_Receive_IT(huart, (uint8_t *) &g_uart1RxByte, 1);
        TestCmdRcvByte(g_uart1RxByte);
    } else if (huart->Instance == USART2) {
        HAL_UART_Receive_IT(huart, (uint8_t *) &g_uart2RxByte, 1);
        AtCommandByteReceived(g_uart2RxByte);
    }
}

int _write(int fd, char *pBuffer, int size)
{
    UNUSED(fd);
    HAL_UART_Transmit(&huart1, (uint8_t *)pBuffer, size, 1000);
    return size;
}

#if defined(__CC_ARM)
struct __FILE {
    int handle;
};

FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f)
{
    UNUSED(f);
    uint8_t data = (uint8_t)ch;
    HAL_UART_Transmit(&huart1, &data, 1, 1000);
    return ch;
}

int fgetc(FILE *f)
{
    UNUSED(f);
    return -1;
}

void _sys_exit(int x)
{
    UNUSED(x);
    while (1) {
    }
}
#endif

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        printf("UART2 Error Callback: ErrorCode=0x%08lX\n", huart->ErrorCode);
    }
}

