#include <Arduino.h>
#include <string.h>
#include "Config.h"
#include "ESP_MultiResetDetector.h"
#include "utils/PersistentDataManager.h"
#include "utils/CustomLED.h"
#include "configMode/ConfigMode.h"
#include "defaultMode/DefaultMode.h"
// #include "Heltec.h"
#include "utils/LogHelper.h"
// #include "utils/DisplayHelper.h"
// #include "utils/StringHelper.h"

CustomLED led(Config::LED_PIN);
Mode *mode = nullptr;
MultiResetDetector *mrd;
bool needsConfig = false;
bool isMultiReset = false;
RTC_FAST_ATTR int bootCount = 0;

void setup()
{
    btStop();
    Serial.begin(115200);
    delay(1000);

    // Heltec.begin(true, false, true);
    LogHelper::log("Main", "Booting...", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    delay(300);

    // Initialize Gateway
    LogHelper::log("Main", "Initialize Gateway", logging::LoggerLevel::LOGGER_LEVEL_INFO);

    // Read MQTT-Host from Filesystem
    if (!PersistentDataManager::readFile(Config::FS_MQTT_HOST_PATH, &Config::MQTT_HOST))
    {
        needsConfig = true;
        LogHelper::log("Main", "No MQTT-Host found", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    }
    else
    {
        LogHelper::log("Main", "MQTT-Host: " + Config::MQTT_HOST, logging::LoggerLevel::LOGGER_LEVEL_INFO);
    }

    // Read MQTT-Port from Filesystem
    String mqtt_port;
    if (!PersistentDataManager::readFile(Config::FS_MQTT_PORT_PATH, &mqtt_port))
    {
        needsConfig = true;
        LogHelper::log("Main", "No MQTT-Port found", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    }
    else
    {
        Config::MQTT_PORT = mqtt_port.toInt();
        LogHelper::log("Main", "MQTT-Port: " + String(Config::MQTT_PORT), logging::LoggerLevel::LOGGER_LEVEL_INFO);
    }

    mrd = new MultiResetDetector(MRD_TIMEOUT, MRD_ADDRESS);
    bootCount++;

    if (bootCount >= 2)
    {
        bootCount = 0;
        // RecentResetFlag löschen um nach deep sleep nicht in ConfigMode zu kommen
        mrd->ResetFromExtern();
    }
    isMultiReset = mrd->detectMultiReset();

    if (!isMultiReset && !needsConfig)
    {
        LogHelper::log("Main", "No Multi Reset Detected => DefaultMode", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        led.turnOn();
        // Wifi-Config laden
        String wifi_ssid;
        String wifi_password;

        bool successSSID = PersistentDataManager::readFile(Config::FS_WIFI_SSID_PATH, &wifi_ssid);
        bool successPassword = PersistentDataManager::readFile(Config::FS_WIFI_PASSWORD_PATH, &wifi_password);
        if (successSSID && successPassword && wifi_ssid.length() != 0)
        {
            // Config vorhanden ==> in default Mode wechseln
            mode = new DefaultMode(&led, wifi_ssid, wifi_password);
            mode->setup();
            return;
        }
    }
    else
    {
        LogHelper::log("Main", "Multi Reset Detected => ConfigMode", logging::LoggerLevel::LOGGER_LEVEL_INFO);

        delay(300);
        led.blink(500);
        mode = new ConfigMode(&led);
        mode->setup();
    }
}

void loop()
{
    led.loop();
    mode->loop();
    mrd->loop();
}