
cmake_minimum_required(VERSION 3.10)
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_STANDARD                11)
set(CMAKE_C_STANDARD_REQUIRED       ON)
set(CMAKE_C_EXTENSIONS              ON)
set(CMAKE_COLOR_DIAGNOSTICS         ON)

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

set(CROSS_COMPILE_PREFIX arm-none-eabi)
set(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_OBJCOPY ${CROSS_COMPILE_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${CROSS_COMPILE_PREFIX}-objdump)
set(CMAKE_SIZE ${CROSS_COMPILE_PREFIX}-size)

project(cipher_node_touch LANGUAGES C CXX)

set(PROJECT_DIR "${CMAKE_SOURCE_DIR}")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(CORE_DIR "${SOURCE_DIR}/core")
set(APPLICATION_DIR "${SOURCE_DIR}/application")
set(FREERTOS_DIR "${SOURCE_DIR}/FreeRTOS")
set(DRIVER_DIR "${SOURCE_DIR}/driver")
set(ST_LIBS_DIR "${SOURCE_DIR}/st_libs")
set(USB_DEVICE_DIR "${SOURCE_DIR}/usb_device")
set(UTILS_DIR "${SOURCE_DIR}/utils")
set(TASKS_DIR "${SOURCE_DIR}/tasks")
set(TEST_DIR "${SOURCE_DIR}/test")
set(MSG_DIR "${SOURCE_DIR}/msg")
set(CM_BACKTRACE_DIR "${SOURCE_DIR}/cm_backtrace")
set(FATFS_DIR "${SOURCE_DIR}/fatfs")
set(COMPONENTS_DIR "${SOURCE_DIR}/components")
set(PORTING_DIR "${SOURCE_DIR}/porting")
set(WIRELESS_DIR "${SOURCE_DIR}/wireless")
set(WIFI_DIR "${SOURCE_DIR}/wifi")
set(UI_DIR "${SOURCE_DIR}/ui")
set(UI_FRAMEWORK_DIR "${UI_DIR}/framework")
set(UI_PAGES_DIR "${UI_DIR}/pages")
set(UI_IMAGES_DIR "${UI_DIR}/images")
set(UI_WIDGETS_DIR "${UI_DIR}/widgets")
set(UI_THEMES_DIR "${UI_DIR}/themes")
set(UI_I18N_DIR "${UI_DIR}/i18n")
set(LVGL_DIR "${PROJECT_DIR}/third/lvgl")
set(CTAES_DIR "${PROJECT_DIR}/third/ctaes")

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -g3")

set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_C_LINK_FLAGS "${TARGET_FLAGS}")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -T \"${SOURCE_DIR}/STM32F405RGTx_FLASH.ld\"")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nano.specs")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--no-warn-rwx-segments")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -u _printf_float")

set(CMAKE_CXX_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lstdc++ -lsupc++ -Wl,--end-group")
# ---- MCU specific flags

set(LV_BUILD_CONF_PATH ${PORTING_DIR}/lv_conf.h CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_DEMOS ON CACHE STRING "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE STRING "" FORCE)
add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)

add_definitions(-DUSE_HAL_DRIVER)
add_definitions(-DSTM32F405xx)

set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)

add_subdirectory(${LVGL_DIR})

include_directories(${CORE_DIR})
include_directories(${APPLICATION_DIR})
include_directories(${USB_DEVICE_DIR}/App)
include_directories(${USB_DEVICE_DIR}/Target)
include_directories(${USB_DEVICE_DIR}/STM32_USB_Device_Library/Core/Inc)
include_directories(${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class)
include_directories(${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/MSC/Inc)
include_directories(${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/CDC/Inc)
include_directories(${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Inc)
include_directories(${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Inc/Legacy)
include_directories(${ST_LIBS_DIR}/CMSIS/Device/ST/STM32F4xx/Include)
include_directories(${ST_LIBS_DIR}/CMSIS/Include)
include_directories(${FREERTOS_DIR})
include_directories(${FREERTOS_DIR}/Source)
include_directories(${FREERTOS_DIR}/Source/include)
include_directories(${FREERTOS_DIR}/CMSIS_RTOS_V2)
include_directories(${FREERTOS_DIR}/Source/portable/GCC/ARM_CM4F)
include_directories(${DRIVER_DIR})
include_directories(${UTILS_DIR})
include_directories(${TASKS_DIR})
include_directories(${TEST_DIR})
include_directories(${MSG_DIR})
include_directories(${CM_BACKTRACE_DIR})
include_directories(${FATFS_DIR})
include_directories(${COMPONENTS_DIR})
include_directories(${COMPONENTS_DIR}/checksum)
include_directories(${COMPONENTS_DIR}/cjson)
include_directories(${PORTING_DIR})
include_directories(${WIRELESS_DIR})
include_directories(${WIFI_DIR})
include_directories(${UI_FRAMEWORK_DIR})
include_directories(${UI_PAGES_DIR})
include_directories(${UI_IMAGES_DIR})
include_directories(${UI_WIDGETS_DIR})
include_directories(${UI_THEMES_DIR})
include_directories(${UI_I18N_DIR})
include_directories(${LVGL_DIR})
include_directories(${CTAES_DIR})


set_property(SOURCE ${CORE_DIR}/startup_stm32f405xx.s PROPERTY LANGUAGE C)
set_property(SOURCE ${SOURCE_DIR}/cm_backtrace/fault_handler/gcc/cmb_fault.S PROPERTY LANGUAGE C)

file(GLOB CORE_FILES
    "${CORE_DIR}/*.c"
)

file(GLOB_RECURSE APPLICATION_FILES
    "${APPLICATION_DIR}/*.c"
)

file(GLOB USBD_APP_FILES
    "${USB_DEVICE_DIR}/App/*.c"
)

file(GLOB USBD_TARGET_FILES
    "${USB_DEVICE_DIR}/Target/*.c"
)

file(GLOB_RECURSE DRIVER_FILES
    "${DRIVER_DIR}/*.c"
)

file(GLOB UTILS_FILES
    "${UTILS_DIR}/*.c"
)

file(GLOB TASKS_FILES
    "${TASKS_DIR}/*.c"
)

file(GLOB TEST_FILES
    "${TEST_DIR}/*.c"
)

file(GLOB MSG_FILES
    "${MSG_DIR}/*.c"
)

file(GLOB CM_BACKTRACE_FILES
    "${CM_BACKTRACE_DIR}/*.c"
)

file(GLOB FATFS_FILES
    "${FATFS_DIR}/*.c"
)

file(GLOB_RECURSE COMPONENTS_FILES
    "${COMPONENTS_DIR}/*.c"
)

file(GLOB PORTING_FILES
    "${PORTING_DIR}/*.c"
)

file(GLOB WIRELESS_FILES
    "${WIRELESS_DIR}/*.c"
)

file(GLOB WIFI_FILES
    "${WIFI_DIR}/*.c"
)

file(GLOB HAL_DRIVER_FILES
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "${ST_LIBS_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c"
)

file(GLOB FREERTOS_DIR_FILES
    "${FREERTOS_DIR}/Source/croutine.c"
    "${FREERTOS_DIR}/Source/event_groups.c"
    "${FREERTOS_DIR}/Source/list.c"
    "${FREERTOS_DIR}/Source/queue.c"
    "${FREERTOS_DIR}/Source/stream_buffer.c"
    "${FREERTOS_DIR}/Source/tasks.c"
    "${FREERTOS_DIR}/Source/timers.c"
    "${FREERTOS_DIR}/Source/portable/MemMang/heap_4.c"
    "${FREERTOS_DIR}/Source/portable/GCC/ARM_CM4F/port.c"
    "${FREERTOS_DIR}/CMSIS_RTOS_V2/cmsis_os2.c"
    "${FREERTOS_DIR}/rtos_expand.c"
)


file(GLOB USBD_FILES
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Core/Src/usbd_core.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/usbd_composite.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_bot.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_data.c"
    "${USB_DEVICE_DIR}/STM32_USB_Device_Library/Class/MSC/Src/usbd_msc_scsi.c"
)


file(GLOB_RECURSE UI_FILES
    "${UI_DIR}/*.c"
)

file(GLOB CTAES_FILES
    "${CTAES_DIR}/ctaes.c"
)


# Suppress third-party LVGL unused-parameter warnings without patching vendor files
target_compile_options(lvgl PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-unused-parameter>)

set(objsrc
    ${CORE_FILES}
    ${APPLICATION_FILES}
    ${USBD_APP_FILES}
    ${USBD_TARGET_FILES}
    ${DRIVER_FILES}
    ${UTILS_FILES}
    ${TASKS_FILES}
    ${TEST_FILES}
    ${MSG_FILES}
    ${CM_BACKTRACE_FILES}
    ${FATFS_FILES}
    ${COMPONENTS_FILES}
    ${PORTING_FILES}
    ${WIRELESS_FILES}
    ${WIFI_FILES}
    ${HAL_DRIVER_FILES}
    ${FREERTOS_DIR_FILES}
    ${USBD_FILES}
    ${UI_FILES}
    ${CTAES_FILES}
    ${CORE_DIR}/startup_stm32f405xx.s
    ${SOURCE_DIR}/cm_backtrace/fault_handler/gcc/cmb_fault.S)
add_executable(${PROJECT_NAME}.elf ${objsrc})
target_link_libraries(${PROJECT_NAME}.elf
lvgl::lvgl
)
set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)
add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
    COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
    COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"
    COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMENT "Invoking: Cross ARM GNU Print Size"
)
