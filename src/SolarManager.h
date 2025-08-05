#ifndef SOLARMANAGER_H
#define SOLARMANAGER_H

#include <Arduino.h>

class SolarManager
{
public:
    SolarManager(uint8_t pinTriac, uint8_t _pinZeroCross, int boilerPower = 2500);
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

    // Constante : durée cible pour que le triac équilibre production et charge
    static constexpr unsigned long regulationDuration = 10000; // 10 secondes par exemple

private:
    uint8_t _pinTriac;
    uint8_t _pinZeroCross;
    float lastPower; // Dernière puissance mesurée
    int loadPower;   // Puissance de la charge
    volatile unsigned long lastZeroCross;
    void handleTimer();
    void handleZeroCross();
};

#endif
