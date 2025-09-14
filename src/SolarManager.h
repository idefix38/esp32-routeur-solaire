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
     * Fonction pour calculer l'heure de lever du soleil et la retourner sous forme de struct tm (par valeur).
     * En cas d'erreur, la structure retournée aura `tm_year == -1`.
     */
    static struct tm calculateSunrise(double latitude, double longitude);

    /**
     * Fonction pour calculer l'heure de coucher du soleil (retour par valeur).
     * En cas d'erreur, la structure retournée aura `tm_year == -1`.
     */
    static struct tm calculateSunset(double latitude, double longitude);

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
};

#endif
