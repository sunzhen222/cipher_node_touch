#ifndef _MQTT_CONNECT_H
#define _MQTT_CONNECT_H

#include "stdint.h"
#include "stdbool.h"

#define MQTT_CONNECT_OK                     0
#define MQTT_CONNECT_ERR_SET_HOST          -1
#define MQTT_CONNECT_ERR_SET_PORT          -2
#define MQTT_CONNECT_ERR_SET_TLS           -3
#define MQTT_CONNECT_ERR_SET_CLIENT_ID     -4
#define MQTT_CONNECT_ERR_SET_USERNAME      -5
#define MQTT_CONNECT_ERR_SET_PASSWORD      -6
#define MQTT_CONNECT_ERR_CONNECT           -7
#define MQTT_CONNECT_ERR_SUBSCRIBE         -8

int32_t ConnectMqtt(void);
bool IsMqttConnected(void);
bool DisconnectMqtt(void);

#endif
