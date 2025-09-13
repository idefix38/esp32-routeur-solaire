#ifndef SOLARMANAGER_H
#define SOLARMANAGER_H

#include <Arduino.h>
#include <math.h>

class SolarManager
{
public:
    SolarManager(uint8_t pinTriac, uint8_t _pinZeroCross);
    void begin();
    float updateRegulation(float power);
    void On();
    void Off();
    /**
     * Fonction pour calculer l'heure de lever du soleil et la retourner sous forme de tm*
     * */
    static tm *calculateSunrise(float latitude, float longitude, int utcOffset, bool isDaylightSaving, struct tm timeinfo);

    /**
     * Fonction pour calculer l'heure de coucher du soleil
     */
    static tm *calculateSunset(float latitude, float longitude, int utcOffset, bool isDaylightSaving, struct tm timeinfo);

    // Accès aux variables utiles
    volatile int delayTriac;
    volatile int powerDelay;

    // Wrappers statiques pour interruptions
    static void IRAM_ATTR onTimerStatic();
    static void IRAM_ATTR onZeroCrossStatic();

    static SolarManager *instance;

private:
    uint8_t _pinTriac;
    uint8_t _pinZeroCross;
    float lastPower;        // Dernière puissance mesurée
    float avgPowerPerPoint; // Ecart moyen de puissance par pourcentage d'ouverture du triac

    volatile unsigned long lastZeroCross;
    void handleTimer();
    void handleZeroCross();
    static int dayOfYear(int day, int month, int year);
    static float convertDegToRad(float angle);
    static float convertRadToDeg(float angle);
};

#endif
