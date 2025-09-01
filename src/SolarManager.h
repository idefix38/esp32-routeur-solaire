#ifndef SOLARMANAGER_H
#define SOLARMANAGER_H

#include <Arduino.h>

class SolarManager
{
public:
    SolarManager(uint8_t pinTriac, uint8_t _pinZeroCross);
    void begin();
    float RegulationProduction(float Power);
    void On();
    void Off();

    // Accès aux variables utiles
    volatile int delayTriac;
    volatile int retard;

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
