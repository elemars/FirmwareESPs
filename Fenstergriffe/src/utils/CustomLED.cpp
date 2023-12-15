#include "utils/CustomLED.h"


CustomLED::CustomLED(uint8_t ledPin) 
    : pin(ledPin),
      blinkActive(false),
      previousMillis(0)
{
    pinMode(this->pin, OUTPUT);
    digitalWrite(this->pin, AUS);
}


void CustomLED::turnOn() {
    this->blinkActive = false;
    digitalWrite(this->pin, AN);
}


void CustomLED::turnOff() {
    this->blinkActive = false;
    digitalWrite(this->pin, this->AUS);
}


void CustomLED::blink(uint16_t interval_ms) {
    this->blinkActive = true; 
    this->blinkIntervall_ms = interval_ms;  
}


void CustomLED::loop() {
    if (blinkActive) {
        uint32_t currentMillis = millis();
        if ((currentMillis - previousMillis) >= blinkIntervall_ms) {
            digitalWrite(pin, !digitalRead(pin));
            previousMillis = currentMillis;
        }
    }
}