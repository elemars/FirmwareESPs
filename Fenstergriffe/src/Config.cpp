#include "Config.h"
#include "Arduino.h"

String Config::MQTT_HOST = "";
int Config::MQTT_PORT = 1883;

// CONSTANTS
const char *Config::AP_SSID = "ESP32_setup";

const char *Config::MQTT_TOPIC = "Fenstermonitoring/";

const char *Config::OTA_PASSWORD = "notusedrightnow";

const char *Config::FS_WIFI_SSID_PATH = "/wifi_ssid.txt";
const char *Config::FS_WIFI_PASSWORD_PATH = "/wifi_password.txt";

const char *Config::FS_MQTT_HOST_PATH = "/mqtt_host.txt";
const char *Config::FS_MQTT_PORT_PATH = "/mqtt_port.txt";
