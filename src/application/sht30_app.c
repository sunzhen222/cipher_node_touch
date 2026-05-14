#include "sht30_app.h"

#include "stdio.h"
#include "ui_msg.h"

void Sht30AppInit(void)
{
    Sht30Init();
}

void Sht30AppRefresh(bool printLog)
{
    Sht30Data_t data;

    if (!Sht30Read(&data)) {
        if (printLog) {
            printf("SHT30 read failed\n");
        }
        return;
    }

    if (printLog) {
        printf("SHT30 T=%d.%dC RH=%u.%u%%\n",
               data.temperatureCx10 / 10,
               data.temperatureCx10 >= 0 ? data.temperatureCx10 % 10 : -(data.temperatureCx10 % 10),
               data.humidityRhx10 / 10,
               data.humidityRhx10 % 10);
    }

    SendUiMsg(UI_MSG_CODE_SHT30, &data, sizeof(data));
}
