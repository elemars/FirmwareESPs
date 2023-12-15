#ifndef MODE_H
#define MODE_H


class CustomLED;

class Mode {
private:

    Mode();

protected:
    CustomLED *led;

public:

    Mode(CustomLED *led)
        :led(led) {}

    virtual void setup() = 0;  

    virtual void loop() = 0;  
};

#endif //MODE_H
