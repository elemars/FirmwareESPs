#include "defaultMode/DefaultMode.h"
#include "utils/CustomLED.h"
#include <Arduino.h>

#include <utility>
#include "ESP_MultiResetDetector.h"
#include "Config.h"
#include "WiFi.h"
#include "utils/Timer.h"
#include "utils/PersistentDataManager.h"
#include "StringArray.h"
#include "utils/LogHelper.h"
#include "ArduinoJson.h"

#define us_To_s_Faktor 1000000U

DefaultMode *DefaultMode::self = nullptr;
MultiResetDetector *mrd1;
bool DefaultMode::isInitialized = false;
bool DefaultMode::isConfigured = false;
bool EXT_WAKEUP = false;
RTC_FAST_ATTR uint64_t PIN_BITMASK = 0x30;

bool DefaultMode::sendMQTTMessage(StateValue value, double voltage)
{
    LogHelper::log("MQTT", "Sending mqtt message", logging::LoggerLevel::LOGGER_LEVEL_INFO);

    DynamicJsonDocument payloadDoc(256);
    payloadDoc["device"] = WiFi.macAddress();
    payloadDoc["state"] = value;
    payloadDoc["voltage"] = voltage;

    String payload;
    serializeJson(payloadDoc, payload);

    bool res = mqtt.publish(Config::MQTT_TOPIC, payload, false, 1);
    if (!res)
    {
        LogHelper::log("MQTT", "Error sending message", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
    }
    return res;
}

DefaultMode::DefaultMode(CustomLED *led, String wifi_ssid, String wifi_password)
    : Mode(led),
      net(),
      mqtt(256),
      wifi_ssid(std::move(wifi_ssid)),
      wifi_password(std::move(wifi_password))
{
    LogHelper::log("DefaultMode", "Starting in default mode", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    self = this;
}

bool DefaultMode::subscribeMQTT()
{
    LogHelper::log("MQTT", "Subscribing to topic: " + String(Config::MQTT_TOPIC), logging::LoggerLevel::LOGGER_LEVEL_INFO);
    bool result = mqtt.subscribe(Config::MQTT_TOPIC, 1);
    if (!result)
    {
        LogHelper::log("MQTT", "Error subscribing to topic: " + String(Config::MQTT_TOPIC) + " - " + mqtt.lastError(),
                       logging::LoggerLevel::LOGGER_LEVEL_ERROR);
    }
    return result;
}

void DefaultMode::connectMQTT()
{
    LogHelper::log("MQTT", "Connecting to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    Timer timer(1);
    while (!mqtt.connect(WiFi.macAddress().c_str()))
    {
        timer.start();
        while (!timer.isExpired())
        {
            led->loop();
            delay(50);
        }
        Serial.print(".");
        delay(50);
    }
    LogHelper::log("MQTT", "Connected to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);

    bool result = this->subscribeMQTT();
    if (result)
    {
        LogHelper::log("MQTT", "Subscribed!", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    }
    else
    {
        LogHelper::log("MQTT", "Could not subscribe to topic - ", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
    }
}

void DefaultMode::connectWifi(const char *ssid, const char *password)
{
    LogHelper::log("WiFi", "Connecting to WiFi with SSID: " + String(ssid), logging::LoggerLevel::LOGGER_LEVEL_INFO);

    WiFiClass::mode(WIFI_STA);
    /*
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("PW: ");
    Serial.println(password);

    Serial.println(WiFi.getHostname());
    Serial.println(WiFi.localIP());
    */
    WiFi.begin(ssid, password);
    Timer timer(1);
    while (WiFiClass::status() != WL_CONNECTED)
    {
        timer.start();
        while (!timer.isExpired())
        {
            led->loop();
            delay(50); // nötig, damit WLAN Verbindung aufgebaut wird
        }
        Serial.print(".");
    }

    LogHelper::log("WiFi", "WiFi connected", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    LogHelper::log("WiFi", "IP address: " + WiFi.localIP().toString(), logging::LoggerLevel::LOGGER_LEVEL_INFO);
    LogHelper::log("WiFi", "MAC address: " + WiFi.macAddress(), logging::LoggerLevel::LOGGER_LEVEL_INFO);
}

void DefaultMode::checkConfigured()
{
    if (!PersistentDataManager::readFile(Config::FS_MQTT_HOST_PATH, &Config::MQTT_HOST))
    {
        LogHelper::log("DefaultMode", "System not initialized.", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        isConfigured = false;
    }
    else
    {
        LogHelper::log("DefaultMode", "System initialized.", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        isConfigured = true;
    }
}

void DefaultMode::workFlowBeforeSleep()

{
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    // wake up source anpassen, je nach Fensterposition
    // verschlossenes Fenster
    if (!digitalRead(REED_PIN_1) && !digitalRead(REED_PIN_2))
    {
        PIN_BITMASK = 0x30; // PIN 4 und 5
        sValue = Locked;
        /* pinMode(REED_PIN_1, INPUT_PULLUP);
        pinMode(REED_PIN_2, INPUT_PULLUP); */
        esp_deep_sleep_enable_gpio_wakeup(PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_HIGH);
        // esp_deep_sleep_enable_gpio_wakeup(REED_PIN_2, ESP_GPIO_WAKEUP_GPIO_HIGH);
    }
    // angelehntes Fenster: Pin 4 LOW, Pin 5 HIGH
    if (!digitalRead(REED_PIN_1) && digitalRead(REED_PIN_2))
    {
        PIN_BITMASK = 0x20; // Pin 5
        sValue = Tilted;
        /* pinMode(REED_PIN_1, INPUT_PULLDOWN);
        pinMode(REED_PIN_2, INPUT_PULLUP); */
        esp_deep_sleep_enable_gpio_wakeup(0x10, ESP_GPIO_WAKEUP_GPIO_HIGH);
        esp_deep_sleep_enable_gpio_wakeup(PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_LOW);
    }
    // gekipptes Fenster: Pin 4 HIGH, Pin 5 LOW
    if (digitalRead(REED_PIN_1) && !digitalRead(REED_PIN_2))
    {
        PIN_BITMASK = 0x10; // Pin 4
        sValue = Closed;
        /* pinMode(REED_PIN_2, INPUT_PULLDOWN);
        pinMode(REED_PIN_1, INPUT_PULLUP); */
        esp_deep_sleep_enable_gpio_wakeup(0x20, ESP_GPIO_WAKEUP_GPIO_HIGH);
        esp_deep_sleep_enable_gpio_wakeup(PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_LOW);
    }
    // geoeffnetes Fenster
    if (digitalRead(REED_PIN_1) && digitalRead(REED_PIN_2))
    {
        PIN_BITMASK = 0x30; // PIN 4 und 5
        sValue = Opened;
        /* pinMode(REED_PIN_1, INPUT_PULLUP);
        pinMode(REED_PIN_2, INPUT_PULLUP); */
        esp_deep_sleep_enable_gpio_wakeup(PIN_BITMASK, ESP_GPIO_WAKEUP_GPIO_LOW);
        // esp_deep_sleep_enable_gpio_wakeup(REED_PIN_2, ESP_GPIO_WAKEUP_GPIO_LOW);
    }

    if (!sendMQTTMessage(sValue, voltage))
    {
        while (!mqtt.connected())
        {
            LogHelper::log("MQTT", "Reconnecting to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);
            this->connectMQTT();
            delay(50);
        }
        sendMQTTMessage(sValue, voltage);
    }

    esp_sleep_enable_timer_wakeup(10 * 60 * us_To_s_Faktor);
}

void DefaultMode::setup()
{
    // delay(500);
    checkConfigured();
    pinMode(REED_PIN_1, INPUT);
    pinMode(REED_PIN_2, INPUT);

    this->connectWifi(wifi_ssid.c_str(), wifi_password.c_str());

    LogHelper::log("MQTT", "Connecting to MQTT Broker: " + Config::MQTT_HOST + ":" + String(Config::MQTT_PORT),
                   logging::LoggerLevel::LOGGER_LEVEL_INFO);
    mqtt.begin(Config::MQTT_HOST.c_str(), Config::MQTT_PORT, net);

    if (isConfigured)
    {
        this->connectMQTT();
    }
    delay(50);
}

void DefaultMode::loop()
{
    while (WiFiClass::status() != WL_CONNECTED)
    {
        LogHelper::log("WiFi", "WiFi disconnected. Reconnecting...", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        WiFi.reconnect();
    }

    while (!mqtt.connected())
    {
        LogHelper::log("MQTT", "Reconnecting to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        this->connectMQTT();
        delay(50);
    }
    // Batteriespannung erfassen und in Volt umwandeln
    voltage = 2.0 * (double)analogReadMilliVolts(BATTERY_PIN) / 1000.0;
    workFlowBeforeSleep();
    delay(100);
    esp_deep_sleep_start();
}