#ifndef MULTIRESETDETECTOR_H
#define MULTIRESETDETECTOR_H
#include "Config.h"

#define ESP8266_MRD_USE_RTC     false   //true
#define ESP_MRD_USE_LITTLEFS    false
#define ESP_MRD_USE_SPIFFS      false
#define ESP_MRD_USE_EEPROM      true
#define MULTIRESETDETECTOR_DEBUG false

// Number of subsequent resets during MRD_TIMEOUT to activate
#define MRD_TIMES               5
// Number of seconds after reset during which a subsequent reset will be considered a multi reset.
#define MRD_TIMEOUT             Config::CONFIG_MODE_DELAY_s

#include "ESP_MultiResetDetector.h"      //https://github.com/khoih-prog/ESP_MultiResetDetector

#endif //MULTIRESETDETECTOR_H