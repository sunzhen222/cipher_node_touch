#ifndef _DEVICE_SETTINGS_H
#define _DEVICE_SETTINGS_H

#include "stdint.h"
#include "stdbool.h"

void DeviceSettingsInit(void);
void SaveDeviceSettings(void);

uint32_t DeviceSettingsGetBrightness(void);
void DeviceSettingsSetBrightness(uint32_t brightness);

uint32_t DeviceSettingsGetLoraChatAvatarColor(void);
void DeviceSettingsSetLoraChatAvatarColor(uint32_t color);

const char *DeviceSettingsGetLoraChatUsername(void);
void DeviceSettingsSetLoraChatUsername(const char *username);

uint32_t DeviceSettingsGetMqttChatAvatarColor(void);
void DeviceSettingsSetMqttChatAvatarColor(uint32_t color);

const char *DeviceSettingsGetMqttChatUsername(void);
void DeviceSettingsSetMqttChatUsername(const char *username);

uint32_t DeviceSettingsGetLoraFreq(void);
void DeviceSettingsSetLoraFreq(uint32_t freq);

uint32_t DeviceSettingsGetDeviceId(void);
void DeviceSettingsSetDeviceId(uint32_t id);

uint32_t DeviceSettingsGetLoraNetId(void);
void DeviceSettingsSetLoraNetId(uint32_t id);

const char *DeviceSettingsGetLoraSecretKey(void);
void DeviceSettingsSetLoraSecretKey(const char *secretKey);

uint32_t DeviceSettingsGetLoraSpreadingFactor(void);
void DeviceSettingsSetLoraSpreadingFactor(uint32_t sf);

uint32_t DeviceSettingsGetLoraBandwidth(void);
void DeviceSettingsSetLoraBandwidth(uint32_t bw);

const char *DeviceSettingsGetMqttBrokerHost(void);
void DeviceSettingsSetMqttBrokerHost(const char *host);

uint32_t DeviceSettingsGetMqttBrokerPort(void);
void DeviceSettingsSetMqttBrokerPort(uint32_t port);

uint32_t DeviceSettingsGetMqttTlsMode(void);
void DeviceSettingsSetMqttTlsMode(uint32_t tlsMode);

const char *DeviceSettingsGetMqttSubscribeTopic(void);
void DeviceSettingsSetMqttSubscribeTopic(const char *topic);

uint32_t DeviceSettingsGetMqttSubscribeQos(void);
void DeviceSettingsSetMqttSubscribeQos(uint32_t qos);

uint32_t DeviceSettingsGetMqttPublishTimeoutMs(void);
void DeviceSettingsSetMqttPublishTimeoutMs(uint32_t timeoutMs);

uint32_t DeviceSettingsGetLockScreenTime(void);
void DeviceSettingsSetLockScreenTime(uint32_t lockScreenTime);

void PrintDeviceSettings(void);

void DeviceSettingsTest(int argc, char *argv[]);

#endif
