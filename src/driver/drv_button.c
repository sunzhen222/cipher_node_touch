#include "drv_button.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "user_delay.h"
#include "user_utils.h"

#define BUTTON_INT_PORT                 GPIOA
#define BUTTON_INT_PIN                  GPIO_PIN_1

#define BUTTON_TIMER_TICK               10
#define SHORT_PRESS_BUTTON_TICK         50
#define LONG_PRESS_BUTTON_TICK          2000

static osTimerId_t g_buttonTickTimer;
static volatile bool g_buttonTimerBusy;
static volatile uint32_t g_buttonPressTick;

static ButtonEventCallbackFunc_t g_shortPressEventCallback;
static ButtonEventCallbackFunc_t g_releaseEventCallback;
static ButtonEventCallbackFunc_t g_longPressEventCallback;

static void ButtonTickTimerFunc(void *argument);

/// @brief Button init.
/// @param
void ButtonInit(void)
{
    g_shortPressEventCallback = NULL;
    g_releaseEventCallback = NULL;
    g_longPressEventCallback = NULL;
    g_buttonTimerBusy = false;
    g_buttonPressTick = 0;

    g_buttonTickTimer = osTimerNew(ButtonTickTimerFunc, osTimerPeriodic, NULL, NULL);
}

/// @brief Register a call back function for the specific button event.
/// @param[in] event The specific button event.
/// @param[in] func Callback function.
void RegisterButtonEvent(ButtonEventType event, ButtonEventCallbackFunc_t func)
{
    switch (event) {
    case BUTTON_EVENT_SHORT_PRESS: {
        g_shortPressEventCallback = func;
    }
    break;
    case BUTTON_EVENT_RELEASE: {
        g_releaseEventCallback = func;
    }
    break;
    case BUTTON_EVENT_LONG_PRESS: {
        g_longPressEventCallback = func;
    }
    break;
    default:
        break;
    }
}

void ButtonIntHandler(void)
{
    if (g_buttonTimerBusy == false) {
        osTimerStart(g_buttonTickTimer, BUTTON_TIMER_TICK);
        g_buttonTimerBusy = true;
    }
}

static void ButtonTickTimerFunc(void *argument)
{
    UNUSED(argument);
    g_buttonPressTick += BUTTON_TIMER_TICK;
    if (HAL_GPIO_ReadPin(BUTTON_INT_PORT, BUTTON_INT_PIN) == GPIO_PIN_SET) {
        //Release button
        osTimerStop(g_buttonTickTimer);
        g_buttonTimerBusy = false;
        if (g_buttonPressTick > SHORT_PRESS_BUTTON_TICK && g_buttonPressTick < LONG_PRESS_BUTTON_TICK) {
            //release event
            if (g_releaseEventCallback) {
                g_releaseEventCallback();
            }
        }
        g_buttonPressTick = 0;
    } else {
        //Press button continually
        if (g_buttonPressTick == SHORT_PRESS_BUTTON_TICK) {
            //Short press event
            printf("short press event\n");
            if (g_shortPressEventCallback) {
                g_shortPressEventCallback();
            }
        } else if (g_buttonPressTick == LONG_PRESS_BUTTON_TICK) {
            //Long press event
            printf("long press event\n");
            if (g_longPressEventCallback) {
                g_longPressEventCallback();
            }
        }
    }
}

/// @brief Get the state if the button is pressed now.
/// @return true-press, false-not press.
bool ButtonPress(void)
{
    return HAL_GPIO_ReadPin(BUTTON_INT_PORT, BUTTON_INT_PIN) == GPIO_PIN_RESET;
}
