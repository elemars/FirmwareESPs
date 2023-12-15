#ifndef CONFIGMODE_H
#define CONFIGMODE_H
#include "Mode.h"
#include "ESPAsyncWebServer.h"
#include <DNSServer.h>

class CustomLED;

class ConfigMode: public Mode {
private:
    DNSServer dnsServer;
    AsyncWebServer server;

    ConfigMode();

    void initAP(const char* ssid);

    void initServer();

public:

    ConfigMode(CustomLED *led);

    void setup() override;

    void loop() override;

};

#endif //CONFIGMODE_H