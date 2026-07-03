#include "page.h"
#include "stdio.h"
#include "lvgl.h"
#include "pages_declare.h"
#include "ui_msg.h"
#include "navigation_bar.h"
#include "user_utils.h"
#include "user_memory.h"
#include "drv_sht30.h"

#define CHART_POINT_COUNT       120     // Show 120 points, one point per second, for 2 minutes of data.
#define CHART_WIDTH             280
#define CHART_HEIGHT            200

typedef struct {
    lv_obj_t *chart;
    lv_obj_t *tempLabel;
    lv_obj_t *humidLabel;
    lv_chart_series_t *tempSeries;
    lv_chart_series_t *humidSeries;
} SensorPageValues_t;


static void SensorPageInit(void);
static void SensorPageDeinit(void);
static void SensorPageMsgHandler(uint32_t code, void *data, uint32_t dataLen);
static void UpdateChartData(Sht30Data_t *data);


Page_t g_sensorPage = {
    .init = SensorPageInit,
    .deinit = SensorPageDeinit,
    .msgHandler = SensorPageMsgHandler,
    .fullScreen = false,
};


static void SensorPageInit(void)
{
    SensorPageValues_t *values = SRAM_MALLOC(sizeof(SensorPageValues_t));
    memset(values, 0, sizeof(SensorPageValues_t));
    lv_obj_set_user_data(GetPageBackground(), values);

    CreateGeneralNavigationBar();

    // Temperature value label.
    values->tempLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->tempLabel, "Temperature: --.- C");
    lv_obj_align(values->tempLabel, LV_ALIGN_TOP_LEFT, 20, 50);
    lv_obj_set_style_text_color(values->tempLabel, lv_color_hex(0xFF0000), 0);

    // Humidity value label.
    values->humidLabel = lv_label_create(GetPageBackground());
    lv_label_set_text(values->humidLabel, "Humidity: --.- %");
    lv_obj_align(values->humidLabel, LV_ALIGN_TOP_RIGHT, -20, 50);
    lv_obj_set_style_text_color(values->humidLabel, lv_color_hex(0x0000FF), 0);

    // Create the chart.
    values->chart = lv_chart_create(GetPageBackground());
    lv_obj_set_size(values->chart, CHART_WIDTH, CHART_HEIGHT);
    lv_obj_align(values->chart, LV_ALIGN_TOP_MID, 0, 90);
    lv_chart_set_type(values->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(values->chart, CHART_POINT_COUNT);

    // Set the Y-axis range to 0-100 so both humidity and temperature are visible.
    lv_chart_set_axis_range(values->chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);

    // Set the division line count.
    lv_chart_set_div_line_count(values->chart, 5, 6);

    // Add the temperature series in red.
    values->tempSeries = lv_chart_add_series(values->chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);

    // Add the humidity series in blue.
    values->humidSeries = lv_chart_add_series(values->chart, lv_color_hex(0x0000FF), LV_CHART_AXIS_PRIMARY_Y);

    // Keep initial points hidden so the first sample is not connected from zero.
    lv_chart_set_all_values(values->chart, values->tempSeries, LV_CHART_POINT_NONE);
    lv_chart_set_all_values(values->chart, values->humidSeries, LV_CHART_POINT_NONE);
}


static void SensorPageDeinit(void)
{
    SensorPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values) {
        SRAM_FREE(values);
    }
}


static void SensorPageMsgHandler(uint32_t code, void *data, uint32_t dataLen)
{
    if (code == UI_MSG_CODE_SHT30 && data != NULL && dataLen == sizeof(Sht30Data_t)) {
        UpdateChartData((Sht30Data_t *)data);
    }
}


static void UpdateChartData(Sht30Data_t *data)
{
    SensorPageValues_t *values = lv_obj_get_user_data(GetPageBackground());
    if (values == NULL || values->chart == NULL) {
        return;
    }

    // Temperature value, converted for one-decimal display.
    int16_t tempInt = data->temperatureCx10 / 10;
    int16_t tempFrac = data->temperatureCx10 >= 0 ? data->temperatureCx10 % 10 : -(data->temperatureCx10 % 10);

    // Humidity value.
    uint16_t humidInt = data->humidityRhx10 / 10;
    uint16_t humidFrac = data->humidityRhx10 % 10;

    // Update value labels.
    char tempStr[64];
    snprintf(tempStr, sizeof(tempStr), "Temperature: %d.%d C", tempInt, tempFrac);
    lv_label_set_text(values->tempLabel, tempStr);

    char humidStr[64];
    snprintf(humidStr, sizeof(humidStr), "Humidity: %u.%u %%", humidInt, humidFrac);
    lv_label_set_text(values->humidLabel, humidStr);

    // Update chart data.
    // Use the actual temperature value directly. Typical values fit in the 0-100 Y-axis range.
    lv_chart_set_next_value(values->chart, values->tempSeries, data->temperatureCx10 / 10);

    // Use the actual humidity percentage value.
    lv_chart_set_next_value(values->chart, values->humidSeries, data->humidityRhx10 / 10);
}
