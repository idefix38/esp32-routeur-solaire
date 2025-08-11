#include "SolarManager.h"

SolarManager *SolarManager::instance = nullptr;

SolarManager::SolarManager(uint8_t _pinTriac, uint8_t _pinZeroCross, int loadPower)
    : _pinTriac(_pinTriac), _pinZeroCross(_pinZeroCross), delayTriac(0), retard(100), loadPower(loadPower)
{
    instance = this;
}

void SolarManager::begin()
{
    pinMode(_pinTriac, OUTPUT);
    pinMode(_pinZeroCross, INPUT);
    digitalWrite(_pinTriac, LOW);
}

void IRAM_ATTR SolarManager::onTimerStatic()
{
    if (instance)
        instance->handleTimer();
}

void IRAM_ATTR SolarManager::onZeroCrossStatic()
{
    if (instance)
        instance->handleZeroCross();
}

/**
 * Méthode executée toutes les 100 microsecondes, pour gérer l'ouverture du triac
 * Passe 100 fois par demi-cycle de la sinusoide
 */
void SolarManager::handleTimer()
{
    delayTriac += 1;
    if (delayTriac > retard && retard < 98)
    {
        digitalWrite(_pinTriac, HIGH);
    }
    else
    {
        digitalWrite(_pinTriac, LOW);
    }
}

/**
 * Méthode executée lors du passage à zéro de la sinusoide du secteur
 */
void SolarManager::handleZeroCross()
{
    if ((millis() - lastZeroCross) > 2)
    {
        delayTriac = 0;
        lastZeroCross = millis();
        digitalWrite(_pinTriac, LOW);
    }
}

/**
 * Régulation du Triac selon la puissance mesurée, retourne la puissance régulée
 */
float SolarManager::RegulationProduction(float power)
{
    lastPower = power; // Mémorise la dernière puissance mesurée

    // La valeur '10' peut être ajustée pour affiner la régulation.
    int ecart = int(power * 10 / loadPower);

    int old_retard = retard;
    retard = retard + ecart;

    if (retard < 0)
        retard = 0;
    if (retard > 100)
        retard = 100;

    float regulatedPower = ((100.0f - retard) / 100.0f) * loadPower;

    return regulatedPower;
}

/**
 * Marche forcée du triac
 */
void SolarManager::On()
{
    retard = 0;
    digitalWrite(_pinTriac, HIGH);
}

/**
 * Stop forcé du triac
 */
void SolarManager::Off()
{
    retard = 100;
    digitalWrite(_pinTriac, LOW);
}