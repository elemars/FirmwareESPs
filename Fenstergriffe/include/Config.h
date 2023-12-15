#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

class Config
{
public:
    static String MQTT_HOST;
    static int MQTT_PORT;

    // Constants
    static const char *AP_SSID;

    static const char *MQTT_TOPIC;

    static const char *OTA_PASSWORD;

    static const char *FS_WIFI_SSID_PATH;
    static const char *FS_WIFI_PASSWORD_PATH;

    static const char *FS_MQTT_HOST_PATH;
    static const char *FS_MQTT_PORT_PATH;

    static const uint8_t LED_PIN = 7U;

    static const uint8_t CONFIG_MODE_DELAY_s = 10;

};

#endif // CONFIG_H