#include "startup.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "usb_device.h"
#include "cm_backtrace.h"
#include "drv_gpio.h"
#include "drv_w25qxx.h"
#include "drv_sys.h"
#include "drv_adc.h"
#include "drv_uart.h"
#include "drv_spi.h"
#include "drv_timer.h"
#include "drv_dma.h"
#include "drv_lcd.h"
#include "drv_i2c.h"
#include "drv_touch.h"
#include "drv_trng.h"
#include "user_utils.h"
#include "user_msg.h"
#include "cmd_task.h"
#include "test_task.h"
#include "usb_task.h"
#include "background_task.h"
#include "ui_task.h"
#include "touch_task.h"
#include "user_fs.h"
#include "device_settings.h"
#include "save_error.h"
#include "hardware_version.h"
#include "software_version.h"
#include "draw_on_lcd.h"
#include "drv_power_switch.h"
#include "drv_button.h"

void Startup(void)
{
    GpioInit();
    Uart1Init();
    Uart2Init();
    AdcInit();
    cm_backtrace_init("cipher_node_touch", GetHardwareVersionString(), GetSoftwareVersionString());
    PrintSystemInfo();
    PowerSwitchInit();
    SpiInit();
    I2cInit();
    TrngInit();
    LcdInit();
    DrawBootLogoOnLcd();
    Timer1Init();
    Timer3Init();

    W25qxx_Init();

    FsMount();
    SaveLastError();
    DeviceSettingsInit();
    ButtonInit();

    //hardware start
    Uart1Start();
    Uart2Start();

    UserMsgInit();
    CreateCmdTask();
    CreateTestTask();
    CreateUsbTask();
    CreateBackgroundTask();
    CreateUiTask();
    CreateTouchTask();
}
