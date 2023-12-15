#ifndef TIMER_H
#define TIMER_H
#include <Arduino.h>

/**
 * @brief Ein Timer Objekt beinhaltet einen Timer, dieser ist nach dem Erzeugen gestoppt und NICHT expired.
 *
 */
class Timer
{
private:
    bool initialized;
    uint32_t previousMillis; // start Zeit
    uint32_t currentMillis;  // aktuelle ms
    bool timeExpired;        // true, wenn der Timer abgelaufen ist
    bool timerActivated;     // true => Timer läuft ganz normal, false => timer läuft nicht weiter und wird als nicht expired angezeigt
    uint32_t stopMillis;     // Zeit, bei der der Timer abgelaufen ist in ms

    /**
     * @brief überprüft, ob der Timer abgelaufen ist
     *
     */
    void loop();

public:
    /**
     * @brief Construct a new Timer object
     *
     * @param time_s die Zeit, die die Timer laufen, bis sie expired sind
     */
    Timer(uint16_t time_s);

    /**
     * @brief Default Konstruktor
     * dieser wird benötigt, wenn ein Array von Timer Objekten erzeugt werden soll. Nach dem Erzeugen muss hier die Funktion
     * setTimeAaufgerufen werden, um das Objekt zu initialisieren
     *
     */
    Timer();

    /**
     * @brief initialisiert das Objekt (muss nur verwendet werden, wennn der Standardkonstruktor verwendet wird)
     *
     * @param time_s die Zeit, die die Timer laufen, bis sie expired sind
     */
    void setTime(uint16_t time_s);

    /**
     * @brief gibt zurück, ob der Timer abgelaufen ist oder nicht
     *
     * @return true
     * @return false
     */
    bool isExpired();

    /**
     * @brief startet den Timer und entfernt die Blockierung, welche durch stopTimer gesetzt wird
     *
     * @return true der Timer war gestoppt (blockiert) und wurde nun wieder gestartet
     * @return false der Timer war nicht gestoppt (blockiert) aber (vielleicht) abgelaufen
     */
    bool start();

    /**
     * @brief stoppt den Timer und setzt ihn auch NICHT expired
     *
     * @return true der Timer war aktiviert und wurde nun deaktiviert
     * @return false der Timer war bereits deaktiviert
     */
    bool stop();

    /**
     * @brief Setzt den Timer manuell auf Zustand "abgelaufen"
     *
     */
    void setExpired();
};
#endif // TIMER_H