/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-rfid-nfc
 */
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <MQTT.h>
#include <utils/LogHelper.h>
#include <json_generator.h>
#include <ArduinoJson.h>
#include <utils/CustomLED.h>
#include <utils/Timer.h>
#include <string>

#define LED_PIN GPIO_NUM_1
#define SS_PIN 5   // ESP32 pin GPIO5
#define RST_PIN 27 // ESP32 pin GPIO27

String MQTT_TOPIC = "Fenstermonitoring/NFC";
String MQTT_HOST = "fensterzentrale.local";
int MQTT_PORT = 1883;
CustomLED led(LED_PIN);
WiFiClient net;
MQTTClient mqtt;
String wifi_ssid = "mars labor";
String wifi_password = "LaborAufDemMars2023!";
String client_id = "NFC_Reader";
MFRC522 rfid(SS_PIN, RST_PIN);

bool sendMQTTMessage(String uidByte, byte lenght)
{
  LogHelper::log("MQTT", "Sending mqtt message", logging::LoggerLevel::LOGGER_LEVEL_INFO);

  DynamicJsonDocument payloadDoc(256);
  payloadDoc["uid"] = uidByte;
  payloadDoc["lenght"] = lenght;

  String payload;
  serializeJson(payloadDoc, payload);

  bool res = mqtt.publish(MQTT_TOPIC, payload, false, 1);
  if (!res)
  {
    LogHelper::log("MQTT", "Error sending message", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
  }
  return res;
}

bool subscribeMQTT()
{
  LogHelper::log("MQTT", "Subscribing to topic: " + String(MQTT_TOPIC), logging::LoggerLevel::LOGGER_LEVEL_INFO);
  bool result = mqtt.subscribe(MQTT_TOPIC, 1);
  if (!result)
  {
    LogHelper::log("MQTT", "Error subscribing to topic: " + String(MQTT_TOPIC) + " - " + mqtt.lastError(),
                   logging::LoggerLevel::LOGGER_LEVEL_ERROR);
  }
  return result;
}

void connectMQTT()
{
  LogHelper::log("MQTT", "Connecting to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);
  Timer timer(1);
  LogHelper::log("MQTT", "Timer set", logging::LoggerLevel::LOGGER_LEVEL_INFO);
  while (!mqtt.connect(client_id.c_str()))
  {
    timer.start();
    LogHelper::log("MQTT", "Timer started", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    while (!timer.isExpired())
    {
      led.loop();
      delay(50);
    }
    Serial.print(".");
    delay(50);
  }
  LogHelper::log("MQTT", "Connected to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);

  bool result = subscribeMQTT();
  if (result)
  {
    LogHelper::log("MQTT", "Subscribed!", logging::LoggerLevel::LOGGER_LEVEL_INFO);
  }
  else
  {
    LogHelper::log("MQTT", "Could not subscribe to topic - ", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
  }
}

void connectWifi(const char *ssid, const char *password)
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
      led.loop();
      delay(50); // nötig, damit WLAN Verbindung aufgebaut wird
    }
    Serial.print(".");
  }

  LogHelper::log("WiFi", "WiFi connected", logging::LoggerLevel::LOGGER_LEVEL_INFO);
  LogHelper::log("WiFi", "IP address: " + WiFi.localIP().toString(), logging::LoggerLevel::LOGGER_LEVEL_INFO);
  LogHelper::log("WiFi", "MAC address: " + WiFi.macAddress(), logging::LoggerLevel::LOGGER_LEVEL_INFO);
}

void setup()
{
  Serial.begin(115200);
  SPI.begin();                                           // init SPI bus
  rfid.PCD_Init();                                       // init MFRC522
  connectWifi(wifi_ssid.c_str(), wifi_password.c_str()); // mit Wifi  verbinden
  LogHelper::log("MQTT", "Connecting to MQTT Broker: " + MQTT_HOST + ":" + String(MQTT_PORT), logging::LoggerLevel::LOGGER_LEVEL_INFO);
  mqtt.begin(MQTT_HOST.c_str(), MQTT_PORT, net);

  connectMQTT();

  delay(50);
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

void loop()
{

  while (WiFiClass::status() != WL_CONNECTED)
  {
    LogHelper::log("WiFi", "WiFi disconnected. Reconnecting...", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    WiFi.reconnect();
  }

  while (!mqtt.connected())
  {
    LogHelper::log("MQTT", "Reconnecting to MQTT Broker", logging::LoggerLevel::LOGGER_LEVEL_INFO);
    connectMQTT();
    delay(50);
  }

  if (rfid.PICC_IsNewCardPresent())
  { // new tag is available
    if (rfid.PICC_ReadCardSerial())
    { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      //Serial.print("RFID/NFC Tag Type: ");
      //Serial.println(rfid.PICC_GetTypeName(piccType));

      // print UID in Serial Monitor in the hex format
      //Serial.print("UID:");
      /*for (int i = 0; i < rfid.uid.size; i++)
      {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }*/
      //Serial.println();
      String byteAsString;
      for (int i = 0; i < rfid.uid.size; i++)
      {
        char hex[3];
        sprintf(hex, "%02X", rfid.uid.uidByte[i]);
        byteAsString += hex;
      }
      Serial.print("UIDasString:");
      Serial.println(byteAsString);
      if (mqtt.connected())
        sendMQTTMessage(byteAsString, rfid.uid.size);
      rfid.PICC_HaltA();      // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
}
