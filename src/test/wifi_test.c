#include "wifi_test.h"
#include "test_cmd.h"
#include "mqtt.h"
#include "wifi_search.h"
#include "stdio.h"
#include "string.h"

static void SearchWifiTest(void)
{
    WifiItem_t wifiHead = {0};
    WifiItem_t *node = &wifiHead;
    uint32_t index = 1;

    printf("searching wifi...\n");
    uint32_t count = SearchWifi(&wifiHead);

    printf("wifi scan count=%lu\n", count);

    while (node != NULL) {
        printf("%lu: ssid=%s ch=%u sec=%s rssi=%d bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
               index,
               node->ssid,
               node->ch,
               WifiSecurityToString(node->security),
               node->rssi,
               node->bssid[0],
               node->bssid[1],
               node->bssid[2],
               node->bssid[3],
               node->bssid[4],
               node->bssid[5]);
        node = node->next;
        index++;
    }

    FreeWifiList(&wifiHead);
}

static void WifiTestCmd(int argc, char *argv[])
{
    if (argc < 1) {
        printf("wifi usage: #wifi:mqtt_connect | #wifi:search\n");
        return;
    }

    if (strcmp(argv[0], "mqtt_connect") == 0) {
        int32_t ret = ConnectMqtt();
        printf("mqtt_connect %s, ret=%ld\n", ret == MQTT_CONNECT_OK ? "ok" : "failed", (long)ret);
        return;
    }

    if (strcmp(argv[0], "search") == 0) {
        SearchWifiTest();
        return;
    }

    printf("unknown wifi test cmd: %s\n", argv[0]);
}

void WifiTestInit(void)
{
    RegisterTestCmd("wifi:", WifiTestCmd);
}
