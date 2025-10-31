#include "solarManager.h"
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

// Portable conversion of a UTC struct tm to time_t without changing TZ.
// Uses civil date formula to compute seconds since epoch for the UTC date/time.
static time_t timegm_compat(const struct tm *tm)
{
    int year = tm->tm_year + 1900;
    int month = tm->tm_mon + 1; // 1..12
    int day = tm->tm_mday;

    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    long long julianDay = day + (153 * m + 2) / 5 + 365LL * y + y / 4 - y / 100 + y / 400 - 32045;
    long long days = julianDay - 2440588LL; // days since 1970-01-01

    long long secs = days * 86400LL + (long long)tm->tm_hour * 3600LL + (long long)tm->tm_min * 60LL + (long long)tm->tm_sec;
    return (time_t)secs;
}

/**
 * @brief Calcule l'heure du lever du soleil en heure locale (retour par valeur)
 */
struct tm SolarManager::calculateSunrise(double latitude, double longitude)
{
    struct tm invalid{};
    invalid.tm_year = -1; // sentinel for error

    struct tm now;
    if (!getLocalTime(&now))
    {
        return invalid;
    }

    int dayOfYear = now.tm_yday + 1;
    double latRad = latitude * M_PI / 180.0;

    // Calcul de l'angle gamma pour l'équation du temps
    double gamma = 2.0 * M_PI / 365.0 * (dayOfYear - 1 + ((now.tm_hour - 12) / 24.0));

    double eqTime = 229.18 * (0.000075 +
                              0.001868 * cos(gamma) -
                              0.032077 * sin(gamma) -
                              0.014615 * cos(2 * gamma) -
                              0.040849 * sin(2 * gamma));

    double decl = 0.006918 -
                  0.399912 * cos(gamma) +
                  0.070257 * sin(gamma) -
                  0.006758 * cos(2 * gamma) +
                  0.000907 * sin(2 * gamma) -
                  0.002697 * cos(3 * gamma) +
                  0.00148 * sin(3 * gamma);

    double ha = acos(cos(M_PI / 180.0 * 90.833) / (cos(latRad) * cos(decl)) - tan(latRad) * tan(decl));

    // Minutes UTC depuis minuit
    double sunriseMinutesUTC = 720.0 - 4.0 * longitude - eqTime - ha * 180.0 / M_PI * 4.0;

    // Build a UTC struct tm for today at 00:00 UTC, then add sunriseMinutesUTC
    time_t now_t = time(nullptr);
    struct tm gm{};
    gmtime_r(&now_t, &gm); // gm holds current UTC date

    struct tm tm_utc_midnight = gm;
    tm_utc_midnight.tm_hour = 0;
    tm_utc_midnight.tm_min = 0;
    tm_utc_midnight.tm_sec = 0;

    // Set hours/min/sec according to sunriseMinutesUTC
    int sunriseHour = (int)(sunriseMinutesUTC / 60.0);
    int sunriseMin = (int)fmod(sunriseMinutesUTC, 60.0);

    struct tm tm_sun_utc = tm_utc_midnight;
    tm_sun_utc.tm_hour = sunriseHour;
    tm_sun_utc.tm_min = sunriseMin;
    tm_sun_utc.tm_sec = 0;

    time_t sunriseTimeUTC = timegm_compat(&tm_sun_utc);

    struct tm sunriseLocal{};
    if (localtime_r(&sunriseTimeUTC, &sunriseLocal) == nullptr)
    {
        return invalid;
    }

    return sunriseLocal;
}

/**
 * @brief Calcule l'heure du coucher du soleil en heure locale (retour par valeur)
 */
struct tm SolarManager::calculateSunset(double latitude, double longitude)
{
    struct tm invalid{};
    invalid.tm_year = -1; // sentinel for error

    struct tm now;
    if (!getLocalTime(&now))
    {
        return invalid;
    }

    int dayOfYear = now.tm_yday + 1;
    double latRad = latitude * M_PI / 180.0;

    double gamma = 2.0 * M_PI / 365.0 * (dayOfYear - 1 + ((now.tm_hour - 12) / 24.0));

    double eqTime = 229.18 * (0.000075 +
                              0.001868 * cos(gamma) -
                              0.032077 * sin(gamma) -
                              0.014615 * cos(2 * gamma) -
                              0.040849 * sin(2 * gamma));

    double decl = 0.006918 -
                  0.399912 * cos(gamma) +
                  0.070257 * sin(gamma) -
                  0.006758 * cos(2 * gamma) +
                  0.000907 * sin(2 * gamma) -
                  0.002697 * cos(3 * gamma) +
                  0.00148 * sin(3 * gamma);

    double ha = acos(cos(M_PI / 180.0 * 90.833) / (cos(latRad) * cos(decl)) - tan(latRad) * tan(decl));

    double sunsetMinutesUTC = 720.0 - 4.0 * longitude - eqTime + ha * 180.0 / M_PI * 4.0;

    // Build a UTC struct tm for today at 00:00 UTC, then add sunsetMinutesUTC
    time_t now_t = time(nullptr);
    struct tm gm{};
    gmtime_r(&now_t, &gm); // gm holds current UTC date

    struct tm tm_utc_midnight = gm;
    tm_utc_midnight.tm_hour = 0;
    tm_utc_midnight.tm_min = 0;
    tm_utc_midnight.tm_sec = 0;

    int sunsetHour = (int)(sunsetMinutesUTC / 60.0);
    int sunsetMin = (int)fmod(sunsetMinutesUTC, 60.0);

    struct tm tm_sun_utc = tm_utc_midnight;
    tm_sun_utc.tm_hour = sunsetHour;
    tm_sun_utc.tm_min = sunsetMin;
    tm_sun_utc.tm_sec = 0;

    time_t sunsetTimeUTC = timegm_compat(&tm_sun_utc);

    struct tm sunsetLocal{};
    if (localtime_r(&sunsetTimeUTC, &sunsetLocal) == nullptr)
    {
        return invalid;
    }

    return sunsetLocal;
}
