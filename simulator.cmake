message(STATUS "simulator")

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

set(SIMULATOR_DIR "${PROJECT_DIR}/simulator")
set(SIMULATOR_LVDRV_DIR "${SIMULATOR_DIR}/lvgl_drv")
set(SIMULATOR_INTERFACES_DIR "${SIMULATOR_DIR}/interfaces")

set(LV_BUILD_CONF_PATH ${PORTING_DIR}/lv_conf.h CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE STRING "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE STRING "" FORCE)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL ON)
set(LV_CONF_BUILD_DISABLE_EXAMPLES ON)
set(LV_CONF_BUILD_DISABLE_DEMOS ON)

add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)
add_definitions(-DSIMULATOR)

include_directories(${PROJECT_DIR})
include_directories(${SIMULATOR_DIR})
include_directories(${SIMULATOR_LVDRV_DIR})
include_directories(${SIMULATOR_INTERFACES_DIR})
include_directories(${APPLICATION_DIR})
include_directories(${UTILS_DIR})
include_directories(${COMPONENTS_DIR}/cjson)
include_directories(${UI_FRAMEWORK_DIR})
include_directories(${UI_PAGES_DIR})
include_directories(${UI_IMAGES_DIR})
include_directories(${UI_WIDGETS_DIR})
include_directories(${UI_THEMES_DIR})
include_directories(${UI_I18N_DIR})
include_directories(${WIFI_DIR})
include_directories(${WIRELESS_DIR})
include_directories(${DRIVER_DIR})
include_directories(${CORE_DIR})
include_directories(${TASKS_DIR})
include_directories(${TEST_DIR})
include_directories(${LVGL_DIR})

file(GLOB SIMULATOR_FILES
    "${SIMULATOR_DIR}/*.c"
)

file(GLOB SIMULATOR_LVDRV_FILES
    "${SIMULATOR_LVDRV_DIR}/*.c"
)

file(GLOB SIMULATOR_INTERFACE_FILES
    "${SIMULATOR_INTERFACES_DIR}/*.c"
)

file(GLOB UI_FRAMEWORK_FILES
    "${UI_FRAMEWORK_DIR}/*.c"
)

file(GLOB UI_PAGES_FILES
    "${UI_PAGES_DIR}/*.c"
)

file(GLOB UI_WIDGETS_FILES
    "${UI_WIDGETS_DIR}/*.c"
)

file(GLOB UI_THEMES_FILES
    "${UI_THEMES_DIR}/*.c"
)

file(GLOB UI_IMAGES_FILES
    "${UI_IMAGES_DIR}/*.c"
)

set(SIMULATOR_APPLICATION_FILES
    "${APPLICATION_DIR}/lora_chat.c"
    "${APPLICATION_DIR}/mqtt_chat.c"
)

set(SIMULATOR_UTILS_FILES
    "${UTILS_DIR}/user_utils.c"
    "${CORE_DIR}/device_settings.c"
)

set(SIMULATOR_COMPONENTS_FILES
    "${COMPONENTS_DIR}/cjson/cJSON.c"
)

add_compile_options(-Wall -Wextra -O2)

set(objsrc
    ${SIMULATOR_FILES}
    ${SIMULATOR_LVDRV_FILES}
    ${SIMULATOR_INTERFACE_FILES}
    ${UI_FRAMEWORK_FILES}
    ${UI_PAGES_FILES}
    ${UI_WIDGETS_FILES}
    ${UI_THEMES_FILES}
    ${UI_IMAGES_FILES}
    ${SIMULATOR_APPLICATION_FILES}
    ${SIMULATOR_UTILS_FILES}
    ${SIMULATOR_COMPONENTS_FILES}
)

add_executable(${PROJECT_NAME} ${objsrc})
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-format)
target_compile_definitions(${PROJECT_NAME} PRIVATE SIMULATOR_FLASH_FILE="${PROJECT_DIR}/simulator_flash.bin")

add_subdirectory(${LVGL_DIR})
target_compile_options(lvgl PRIVATE $<$<COMPILE_LANGUAGE:C>:-Wno-unused-parameter>)

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_compile_options(${PROJECT_NAME} PUBLIC ${SDL2_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} lvgl::lvgl SDL2)
