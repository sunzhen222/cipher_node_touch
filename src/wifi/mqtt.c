#include "mqtt.h"
#include "at_command.h"
#include "user_assert.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os2.h"

#define MQTT_BROKER_HOST             "t1bf11cf.ala.cn-shenzhen.emqxsl.cn"
#define MQTT_BROKER_PORT             8883
#define MQTT_TLS_MODE                2
#define MQTT_CLIENT_ID_PREFIX        "cipher_node_touch_board_"
#define MQTT_AUTH_PREFIX             "CipherNodeTouch_"
#define MQTT_SUBSCRIBE_TOPIC         "testtopic/chat"
#define MQTT_SUBSCRIBE_QOS           0
#define MQTT_PUBLISH_TIMEOUT_MS      5000

static bool g_mqttConnected = false;

static void BuildMqttClientId(char *buffer, size_t bufferSize)
{
    uint32_t uid0 = HAL_GetUIDw0();

    snprintf(buffer, bufferSize, "%s%08lX", MQTT_CLIENT_ID_PREFIX, uid0);
}

static void BuildMqttAuthString(char *buffer, size_t bufferSize)
{
    uint32_t uid0 = HAL_GetUIDw0();
    uint32_t uid1 = HAL_GetUIDw1();
    uint32_t uid2 = HAL_GetUIDw2();

    // Keep UID formatting aligned with about_page, but concatenate without spaces.
    snprintf(buffer, bufferSize, "%s%08lX%08lX%08lX", MQTT_AUTH_PREFIX, uid0, uid1, uid2);
}

bool IsMqttConnected(void)
{
    return g_mqttConnected;
}

int32_t ConnectMqtt(void)
{
    int32_t ret = MQTT_CONNECT_OK;
    char command[AT_COMMAND_MAX_LENGTH];
    char clientId[64];
    char auth[96];

    BuildMqttClientId(clientId, sizeof(clientId));
    BuildMqttAuthString(auth, sizeof(auth));

    AtCommandLock();

    do {
        if (g_mqttConnected) {
            ret = MQTT_CONNECT_OK;
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=1,%s", MQTT_BROKER_HOST);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_HOST;
            printf("ConnectMqtt failed: set broker host, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=2,%d", MQTT_BROKER_PORT);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_PORT;
            printf("ConnectMqtt failed: set broker port, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=3,%d", MQTT_TLS_MODE);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_TLS;
            printf("ConnectMqtt failed: set TLS mode, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=4,%s", clientId);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_CLIENT_ID;
            printf("ConnectMqtt failed: set client id, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=5,%s", auth);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_USERNAME;
            printf("ConnectMqtt failed: set username, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTT=6,%s", auth);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SET_PASSWORD;
            printf("ConnectMqtt failed: set password, ret=%ld\n", (long)ret);
            break;
        }

        if (!SendAtCommandWait("AT+MQTT", "+EVENT:MQTT_CONNECT", "ERROR", 10000)) {
            ret = MQTT_CONNECT_ERR_CONNECT;
            printf("ConnectMqtt failed: connect broker, ret=%ld\n", (long)ret);
            break;
        }

        snprintf(command, sizeof(command), "AT+MQTTSUB=%s,%d", MQTT_SUBSCRIBE_TOPIC, MQTT_SUBSCRIBE_QOS);
        if (!SendAtCommandWait(command, "OK", "ERROR", 5000)) {
            ret = MQTT_CONNECT_ERR_SUBSCRIBE;
            printf("ConnectMqtt failed: subscribe topic, ret=%ld\n", (long)ret);
            break;
        }

        ret = MQTT_CONNECT_OK;
        g_mqttConnected = true;
    } while (0);
    if (ret != MQTT_CONNECT_OK) {
        SendAtCommand("AT+MQTTDISCONN");
        g_mqttConnected = false;
    }
    AtCommandUnlock();
    return ret;
}

bool DisconnectMqtt(void)
{
    bool disconnectOk;

    AtCommandLock();
    disconnectOk = SendAtCommandWait("AT+MQTTDISCONN", "OK", "ERROR", 5000);
    if (disconnectOk) {
        g_mqttConnected = false;
    }
    AtCommandUnlock();

    return disconnectOk;
}

bool PublishMqtt(const char *topic, uint8_t qos, bool retained, const char *payload)
{
    bool publishOk;
    char command[AT_COMMAND_MAX_LENGTH];

    if (topic == NULL || payload == NULL || topic[0] == '\0') {
        return false;
    }
    size_t payloadLength = strlen(payload);

    snprintf(command,
             sizeof(command),
             "AT+MQTTPUBRAW=%s,%u,%u,%u",
             topic,
             qos,
             retained ? 1 : 0,
             payloadLength);

    AtCommandLock();
    ClearReceivedAtCommand();
    SendAtCommand(command);
    osDelay(100);
    publishOk = SendAtCommandWait(payload, "OK", "ERROR", MQTT_PUBLISH_TIMEOUT_MS);
    AtCommandUnlock();

    return publishOk;
}
