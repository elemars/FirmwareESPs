#include "configMode/ConfigMode.h"
#include "utils/CustomLED.h"
#include "utils/Timer.h"
#include "Config.h"
#include <LittleFS.h>
#include "utils/PersistentDataManager.h"
#include "utils/LogHelper.h"
// #include "utils/DisplayHelper.h"

String ssid;
String password;
bool restart = false;

class CaptiveRequestHandler : public AsyncWebHandler
{
private:
    static void notFound(AsyncWebServerRequest *request)
    {
        request->send(404, "text/plain", "Not found");
    }

    static void styleCSS(AsyncWebServerRequest *request)
    {
        request->send(LittleFS, "/www/style.css", "text/css");
    }

    static void submitForm(AsyncWebServerRequest *request)
    {
        bool success = true;
        bool writeReceiver = false;

        if (request->hasParam("ssid", true))
        {
            LogHelper::log("ConfigMode", "SSID: " + request->getParam("ssid", true)->value(),
                           logging::LoggerLevel::LOGGER_LEVEL_INFO);
            if (!PersistentDataManager::writeFile(Config::FS_WIFI_SSID_PATH,
                                                  request->getParam("ssid", true)->value().c_str()))
            {
                success = false;
            }
        }
        else
        {
            success = false;
        }

        if (request->hasParam("password", true))
        {
            LogHelper::log("ConfigMode", "Password: " + request->getParam("password", true)->value(),
                           logging::LoggerLevel::LOGGER_LEVEL_INFO);
            if (!PersistentDataManager::writeFile(Config::FS_WIFI_PASSWORD_PATH,
                                                  request->getParam("password", true)->value().c_str()))
            {
                success = false;
            }
        }

        if (request->hasParam("mqtt_host", true))
        {
            LogHelper::log("ConfigMode", "MQTT Host: " + request->getParam("mqtt_host", true)->value(),
                           logging::LoggerLevel::LOGGER_LEVEL_INFO);
            if (!PersistentDataManager::writeFile(Config::FS_MQTT_HOST_PATH,
                                                  request->getParam("mqtt_host", true)->value().c_str()))
            {
                success = false;
            }
        }
        else
        {
            success = false;
        }
        if (request->hasParam("mqtt_port", true))
        {
            LogHelper::log("ConfigMode", "MQTT Port: " + request->getParam("mqtt_port", true)->value(),
                           logging::LoggerLevel::LOGGER_LEVEL_INFO);
            if (!PersistentDataManager::writeFile(Config::FS_MQTT_PORT_PATH,
                                                  request->getParam("mqtt_port", true)->value().c_str()))
            {
                success = false;
            }
        }
        else
        {
            success = false;
        }

        if (success)
        {
            restart = true;
            return request->send(LittleFS, "/www/response_success.html", "text/html", false);
        }
        request->send(LittleFS, "/www/response_error.html", "text/html", false);
    }

    static void index(AsyncWebServerRequest *request)
    {
        LogHelper::log("ConfigMode", "Serving index.html", logging::LoggerLevel::LOGGER_LEVEL_INFO);

        String wifi_ssid;
        String result;
        if (!PersistentDataManager::readFile("/www/index.html", &result))
        {
            LogHelper::log("ConfigMode", "Error reading index.html", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
            request->send(LittleFS, "/www/response_error.html", "text/html", false);
            return;
        }
        if (!PersistentDataManager::readFile(Config::FS_WIFI_SSID_PATH, &wifi_ssid))
        {
            LogHelper::log("ConfigMode", "Error reading wifi_ssid", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
            wifi_ssid = "";
        }

        result.replace("{{mqtt_host}}", Config::MQTT_HOST);
        result.replace("{{mqtt_port}}", String(Config::MQTT_PORT));
        result.replace("{{ssid}}", wifi_ssid);

        request->send(200, "text/html", result);
    }

public:
    CaptiveRequestHandler(AsyncWebServer *server)
    {
        server->on("/style.css", HTTP_GET, styleCSS);

        server->on("/", HTTP_POST, submitForm);

        server->on("/", HTTP_GET, index);

        server->on("/hotspot-detect.html", HTTP_GET, index);
    }

    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        // request->addInterestingHeader("ANY");
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        // hier wenden alle Anfragen, die nicht den im Konstruktor definierten Pfaden entsprechen bearbeitet
        notFound(request);
    }
};

ConfigMode::ConfigMode(CustomLED *led)
    : Mode(led),
      dnsServer(),
      server(80)
{
    LogHelper::log("ConfigMode", "ConfigMode", logging::LoggerLevel::LOGGER_LEVEL_INFO);
}

void ConfigMode::initAP(const char *ssid)
{
    LogHelper::log("ConfigMode", "Initialize AP", logging::LoggerLevel::LOGGER_LEVEL_INFO);

    WiFiClass::mode(WIFI_AP);
    WiFi.softAP(ssid, nullptr);

    const byte DNS_PORT = 53;
    // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

    IPAddress IP = WiFi.softAPIP();
    LogHelper::log("ConfigMode", "AP IP address: " + IP.toString(), logging::LoggerLevel::LOGGER_LEVEL_INFO);
}

void ConfigMode::initServer()
{
    if (!LittleFS.begin())
    {
        LogHelper::log("ConfigMode", "LittleFS Mount Failed", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        return;
    }
    // für die normalen Anfragen vom Webbrowser (serveStatic ist performanter als der captiveRequestHandler)
    //    server.serveStatic("/", LittleFS, "/www/").setDefaultFile("index.html");
    // für alle weiteren Anfragen (speziell die vom CaptivePortal)
    server.addHandler(new CaptiveRequestHandler(&server)).setFilter(ON_AP_FILTER); // only when requested from AP
    server.begin();
    LogHelper::log("ConfigMode", "Wifi Manager Server started", logging::LoggerLevel::LOGGER_LEVEL_INFO);
}

void ConfigMode::setup()
{
    this->initAP(Config::AP_SSID);
    // OTA::setup(Config::OTA_PASSWORD);
    this->initServer();
}

void ConfigMode::loop()
{
    dnsServer.processNextRequest();
    // OTA::loop();

    Timer timer(180);
    timer.start();
    if (timer.isExpired())
    {
        LogHelper::log("ConfigMode", "Timeout was reached.", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        restart = true;
    }

    if (restart)
    {
        LogHelper::log("ConfigMode", "Restarting...", logging::LoggerLevel::LOGGER_LEVEL_INFO);
        delay(1000);
        ESP.restart();
    }
}