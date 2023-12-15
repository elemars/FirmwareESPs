#include "utils/Timer.h"
#include "utils/LogHelper.h"

Timer::Timer(uint16_t  time_s)
 :  initialized(true),
    previousMillis(0),
    currentMillis(0),
    timeExpired(false),
    timerActivated(false),
    stopMillis(time_s * 1000)
{
}



Timer::Timer()
 :  initialized(false),
    previousMillis(0),
    currentMillis(0),
    timeExpired(false),
    timerActivated(false),
    stopMillis(0)
{
}


void Timer::setTime(uint16_t time_s) {
    stopMillis = time_s * 1000;
    initialized = true;
}


/**
 * überprüft ob ein Timer abgelaufen ist (muss von der Loop in main.cpp aufgerufen werden!)
*/
void Timer::loop()
{
    if (initialized) {
        //überprüfen ob Timer abgelaufen ist
        if (!timeExpired)
        {
            //timer läuft noch ==> updaten
            currentMillis = millis();
            if ((currentMillis - previousMillis) >= stopMillis)
            {
                timeExpired = true;
            }
        }
    }
}


bool Timer::start()
{
    if (initialized)
    {
        if (!timerActivated) {
            timeExpired = false;
            timerActivated = true;
            //Startzeit festelgen
            previousMillis = millis();
            return true;
        }
        else {
            timeExpired = false;
            previousMillis = millis();
            return false;
        }
    }
    else
    {
        LogHelper::log("Timer", "Timer Objekt wird verwendet, obwohl es nicht initialisiert ist!", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        abort();
    }
}


bool Timer::stop() {
    if (initialized)
    {
        if (timerActivated) {
            timerActivated = false;
            timeExpired = false;
            return true;
        }
        return false;
    }
    else
    {
        LogHelper::log("Timer", "Timer Objekt wird verwendet, obwohl es nicht initialisiert ist!", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        abort();
    }
}


void Timer::setExpired()
{
    if (initialized)
    {
        timeExpired = true;
        timerActivated = true;
    }
    else
    {
        LogHelper::log("Timer", "Timer Objekt wird verwendet, obwohl es nicht initialisiert ist!", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        abort();
    }
}


bool Timer::isExpired()
{
    if (initialized)
    {
        loop();
        if (!timerActivated || !timeExpired) {
            return false;
        }
        else 
        {
            return true;
        }
    }
    else
    {
        LogHelper::log("Timer", "Timer Objekt wird verwendet, obwohl es nicht initialisiert ist!", logging::LoggerLevel::LOGGER_LEVEL_ERROR);
        abort();
    }
}