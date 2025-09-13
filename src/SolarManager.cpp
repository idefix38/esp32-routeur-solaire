#include "SolarManager.h"
#include "Arduino.h"

SolarManager *SolarManager::instance = nullptr;

SolarManager::SolarManager(uint8_t _pinTriac, uint8_t _pinZeroCross)
    : _pinTriac(_pinTriac), _pinZeroCross(_pinZeroCross), delayTriac(0), powerDelay(100), avgPowerPerPoint(0)
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
    if (delayTriac > powerDelay && powerDelay < 98)
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
 * Régulation du Triac selon la puissance mesurée, retourne le pourcentage d'ouverture du triac
 */
float SolarManager::updateRegulation(float power)
{
    lastPower = power; // Mémorise la dernière puissance mesurée

    // La valeur '2000' correspond à la puissance max du chauffe-eau. A rendre configurable.
    int powerDifference = int(power * 100 / 2000);

    if (powerDifference != 0 && powerDifference < 98)
    {
        float powerPerPoint = power / powerDifference;
        // Calcul de la moyenne mobile (puissance / % d'ouverture)
        const float alpha = 0.05; // Facteur de lissage = 5%
        avgPowerPerPoint = alpha * powerPerPoint + (1.0f - alpha) * avgPowerPerPoint;
    }

    if (abs(power) < avgPowerPerPoint && power < 0)
    {
        // Si la puissance mesurée est inférieure à la puissance moyenne par point
        // On ne change pas le retard  = stabilisation avec une légère suproduction
    }
    else if (abs(power) < avgPowerPerPoint && power > 0)
    {
        // Légère surconsommation  on augmente le retard de 1 pour diminuer la puissance
        powerDelay = powerDelay + 1;
    }
    else
    {
        // Ajustement du retard
        powerDelay = powerDelay + powerDifference;
    }

    if (powerDelay < 0)
        powerDelay = 0;
    if (powerDelay > 100)
        powerDelay = 100;

    float triacOpeningPercentage = 100.0f - powerDelay;

    return triacOpeningPercentage;
}

/**
 * Marche forcée du triac
 */
void SolarManager::On()
{
    powerDelay = 0;
    digitalWrite(_pinTriac, HIGH);
}

/**
 * Stop forcé du triac
 */
void SolarManager::Off()
{
    powerDelay = 100;
    digitalWrite(_pinTriac, LOW);
}

/**
 *  Fonction utilitaire pour calculer le jour de l'année
 **/
int SolarManager::dayOfYear(int day, int month, int year)
{
    int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
    {
        daysInMonth[2] = 29;
    }
    int doy = 0;
    for (int i = 1; i < month; ++i)
    {
        doy += daysInMonth[i];
    }
    doy += day;
    return doy;
}

float SolarManager::convertDegToRad(float angle)
{
    return angle * PI / 180.0;
}

float SolarManager::convertRadToDeg(float angle)
{
    return angle * 180.0 / PI;
}

tm *SolarManager::calculateSunrise(float latitude, float longitude, int utcOffset, bool isDaylightSaving, struct tm timeinfo)
{
    // Calcul de N basé sur l'heure et la date UTC
    int doy = dayOfYear(timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    float N = doy + (timeinfo.tm_hour / 24.0);

    float L = fmod(280.460 + 0.98564736 * N, 360.0);
    float g = fmod(357.528 + 0.98560028 * N, 360.0);
    float lambda = L + 1.915 * sin(convertDegToRad(g)) + 0.020 * sin(convertDegToRad(2 * g));
    float delta = asin(sin(convertDegToRad(lambda)) * sin(convertDegToRad(23.45)));
    float cosH = (sin(convertDegToRad(-0.58)) - sin(convertDegToRad(latitude)) * sin(delta)) / (cos(convertDegToRad(latitude)) * cos(delta));

    if (cosH > 1 || cosH < -1)
        return nullptr;

    float H = convertRadToDeg(acos(cosH));

    // Calcul de l'heure en temps solaire local
    float sunriseTimeSolar = 12.0 - H / 15.0;

    // Conversion en heure locale
    float sunriseLocal = sunriseTimeSolar - (longitude / 15.0) + utcOffset;
    if (isDaylightSaving)
    {
        sunriseLocal += 1.0;
    }

    tm *result = new tm;
    *result = timeinfo;
    result->tm_hour = (int)floor(sunriseLocal);
    result->tm_min = (int)floor((sunriseLocal - result->tm_hour) * 60);
    result->tm_sec = 0;

    time_t t = mktime(result);
    localtime_r(&t, result);
    return result;
}

tm *SolarManager::calculateSunset(float latitude, float longitude, int utcOffset, bool isDaylightSaving, struct tm timeinfo)
{
    int doy = dayOfYear(timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    float N = doy + (timeinfo.tm_hour / 24.0);
    float L = fmod(280.460 + 0.98564736 * N, 360.0);
    float g = fmod(357.528 + 0.98560028 * N, 360.0);
    float lambda = L + 1.915 * sin(convertDegToRad(g)) + 0.020 * sin(convertDegToRad(2 * g));
    float delta = asin(sin(convertDegToRad(lambda)) * sin(convertDegToRad(23.45)));
    float cosH = (sin(convertDegToRad(-0.58)) - sin(convertDegToRad(latitude)) * sin(delta)) / (cos(convertDegToRad(latitude)) * cos(delta));

    if (cosH > 1 || cosH < -1)
        return nullptr;

    float H = convertRadToDeg(acos(cosH));
    float sunsetTimeSolar = 12.0 + H / 15.0;

    float sunsetLocal = sunsetTimeSolar - (longitude / 15.0) + utcOffset;
    if (isDaylightSaving)
    {
        sunsetLocal += 1.0;
    }

    tm *result = new tm;
    *result = timeinfo;
    result->tm_hour = (int)floor(sunsetLocal);
    result->tm_min = (int)floor((sunsetLocal - result->tm_hour) * 60);
    result->tm_sec = 0;

    time_t t = mktime(result);
    localtime_r(&t, result);
    return result;
}