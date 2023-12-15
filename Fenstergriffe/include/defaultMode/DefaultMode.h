#ifndef DEFAULTMODE_H
#define DEFAULTMODE_H
#include <Arduino.h>
#include "Mode.h"
#include <WiFiClient.h>
#include "MQTT.h"
#include <memory>

class CustomLED;

enum StateValue
{
    Locked,
    Closed,
    Opened,
    Tilted
};

class DefaultMode : public Mode
{
private:
    WiFiClient net;
    MQTTClient mqtt;
    String wifi_ssid;
    String wifi_password;
    static bool isConfigured;
    static bool isInitialized;

    static DefaultMode *self;

    DefaultMode();

    static void setWiFiHostname(const char *hostname);
    void connectWifi(const char *ssid, const char *password);
    void connectMQTT();
    bool subscribeMQTT();
    static void checkConfigured();
    void workFlowBeforeSleep();

    static const uint8_t REED_PIN_1 = 4;
    static const uint8_t REED_PIN_2 = 5;
    static const uint8_t BATTERY_PIN = 3;

    double voltage;
    StateValue sValue = Opened;

public:
    DefaultMode(CustomLED *led, String wifi_ssid, String wifi_password);

    void setup() override;

    void loop() override;

    bool sendMQTTMessage(StateValue value, double voltage);

    static bool isPairing;

    void setClockSource(MQTTClientClockSource);
};

#endif // DEFAULTMODE_H