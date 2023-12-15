#ifndef CustomLED_H
#define CustomLED_H
#include <Arduino.h>


class CustomLED {
private:
    const uint8_t pin;  
    bool blinkActive;
    uint32_t previousMillis;
    uint16_t blinkIntervall_ms; 
    
    //Konstanten
    const uint8_t AN = 0;
    const uint8_t AUS = 1;

public:
    CustomLED(uint8_t ledPin);

    /**
     * @brief schaltet die LED an
     * 
     */
    void turnOn();

    /**
     * @brief schaltet die LED aus
     * 
     */
    void turnOff();

    /**
     * @brief lässt die LED im mitgegebenen Interval blinken
     * 
     * @param interval_ms 
     */
    void blink(uint16_t interval_ms);


    /**
     * @brief überprüft ob Timer abgelaufen ist und lässt LED blinken
     * 
     */
    void loop();

};

#endif //LED_H