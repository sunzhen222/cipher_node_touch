#ifndef _DEVICE_SETTINGS_H
#define _DEVICE_SETTINGS_H

#include "stdint.h"
#include "stdbool.h"

void DeviceSettingsInit(void);
void SaveDeviceSettings(void);

uint32_t DeviceSettingsGetBrightness(void);
void DeviceSettingsSetBrightness(uint32_t brightness);

uint32_t DeviceSettingsGetSpeedCtrlMode(void);
void DeviceSettingsSetSpeedCtrlMode(uint32_t mode);

uint32_t DeviceSettingsGetSpeedCtrlSmooth(void);
void DeviceSettingsSetSpeedCtrlSmooth(uint32_t smooth);

uint32_t DeviceSettingsGetWidgetColor(void);
void DeviceSettingsSetWidgetColor(uint32_t color);

uint32_t DeviceSettingsGetLanguage(void);
void DeviceSettingsSetLanguage(uint32_t language);

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

void PrintDeviceSettings(void);

void DeviceSettingsTest(int argc, char *argv[]);

#endif
