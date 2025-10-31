#include "configManager.h"
#include <Arduino.h>
#include "solarManager.h"
#include <time.h>
// Constructeur
ConfigManager::ConfigManager()
{
}

// Sauvegarde la configuration dans la mémoire flash
bool ConfigManager::saveConfig(const Config &config)
{
    Serial.println("[-] Ecriture de la mémoire flash ...");
    _preferences.begin("config", false);

    // Wifi
    _preferences.putString("w.ssid", config.wifi.ssid.c_str());
    _preferences.putString("w.pass", config.wifi.password.c_str());

    // Mqtt
    _preferences.putString("m.serv", config.mqtt.server.c_str());
    _preferences.putInt("m.port", config.mqtt.port);
    _preferences.putString("m.user", config.mqtt.username.c_str());
    _preferences.putString("m.pass", config.mqtt.password.c_str());
    _preferences.putString("m.topic", config.mqtt.topic.c_str());

    // ShellyEm
    _preferences.putString("sh.ip", config.shellyEm.ip.c_str());
    _preferences.putString("sh.chan", config.shellyEm.channel.c_str());

    // Solar
    _preferences.putFloat("so.lat", config.solar.latitude);
    _preferences.putFloat("so.lon", config.solar.longitude);
    _preferences.putString("so.tz", config.solar.timeZone.c_str());

    // Boiler
    _preferences.putString("b.mode", config.boiler.mode.c_str());
    _preferences.putInt("b.temp", config.boiler.temperature);
    _preferences.putUInt("b.p.size", config.boiler.periods.size());
    for (size_t i = 0; i < config.boiler.periods.size(); ++i)
    {
        std::string baseKey = "b.p." + std::to_string(i);
        _preferences.putInt((baseKey + ".s").c_str(), config.boiler.periods[i].start);
        _preferences.putInt((baseKey + ".e").c_str(), config.boiler.periods[i].end);
        _preferences.putString((baseKey + ".m").c_str(), config.boiler.periods[i].mode.c_str());
        // Set true if sunrise==config.boiler.periods[i].start
        _preferences.putBool((baseKey + ".sr").c_str(), config.boiler.periods[i].startSunrise);
        _preferences.putBool((baseKey + ".ss").c_str(), config.boiler.periods[i].startSunset);
        // Set true if sunset==config.boiler.periods[i].end
        _preferences.putBool((baseKey + ".er").c_str(), config.boiler.periods[i].endSunrise);
        _preferences.putBool((baseKey + ".es").c_str(), config.boiler.periods[i].endSunset);
    }

    _preferences.end();
    return true;
}

// Charge la configuration depuis la mémoire flash
Config ConfigManager::loadConfig()
{
    Config config;
    Serial.println("[-] Lecture de la mémoire flash ...");
    _preferences.begin("config", true);

    // Wifi
    config.wifi.ssid = _preferences.getString("w.ssid", "").c_str();
    config.wifi.password = _preferences.getString("w.pass", "").c_str();

    // Mqtt
    config.mqtt.server = _preferences.getString("m.serv", "").c_str();
    config.mqtt.port = _preferences.getInt("m.port", 1883);
    config.mqtt.username = _preferences.getString("m.user", "").c_str();
    config.mqtt.password = _preferences.getString("m.pass", "").c_str();
    config.mqtt.topic = _preferences.getString("m.topic", "").c_str();

    // ShellyEm
    config.shellyEm.ip = _preferences.getString("sh.ip", "").c_str();
    config.shellyEm.channel = _preferences.getString("sh.chan", "0").c_str();

    // Solar
    config.solar.latitude = _preferences.getFloat("so.lat", 48.8566);
    config.solar.longitude = _preferences.getFloat("so.lon", 2.3522);
    config.solar.timeZone = _preferences.getString("so.tz", "Europe/Paris").c_str();

    struct tm sunrise = SolarManager::calculateSunrise(config.solar.latitude, config.solar.longitude);
    struct tm sunset = SolarManager::calculateSunset(config.solar.latitude, config.solar.longitude);
    config.solar.sunRiseMinutes = sunrise.tm_hour * 60 + sunrise.tm_min;
    config.solar.sunSetMinutes = sunset.tm_hour * 60 + sunset.tm_min;

    // Boiler
    config.boiler.mode = _preferences.getString("b.mode", "auto").c_str();
    config.boiler.temperature = _preferences.getInt("b.temp", 50);
    size_t boilerPeriodsSize = _preferences.getUInt("b.p.size", 0);
    config.boiler.periods.clear();
    for (size_t i = 0; i < boilerPeriodsSize; ++i)
    {
        Period p;
        std::string baseKey = "b.p." + std::to_string(i);

        // Start période time
        if (_preferences.getBool((baseKey + ".sr").c_str()))
        {
            p.start = (sunrise.tm_hour * 60 + sunrise.tm_min);
        }
        else if (_preferences.getBool((baseKey + ".ss").c_str()))
        {
            p.start = (sunset.tm_hour * 60 + sunset.tm_min);
        }
        else
        {
            p.start = _preferences.getInt((baseKey + ".s").c_str(), 0);
        }

        // End periode time
        if (_preferences.getBool((baseKey + ".es").c_str()))
        {
            p.end = (sunset.tm_hour * 60 + sunset.tm_min);
        }
        else if (_preferences.getBool((baseKey + ".er").c_str()))
        {
            p.end = (sunrise.tm_hour * 60 + sunrise.tm_min);
        }
        else
        {
            p.end = _preferences.getInt((baseKey + ".e").c_str(), 0);
        }

        p.startSunrise = _preferences.getBool((baseKey + ".sr").c_str(), false);
        p.startSunset = _preferences.getBool((baseKey + ".ss").c_str(), false);
        p.endSunrise = _preferences.getBool((baseKey + ".er").c_str(), false);
        p.endSunset = _preferences.getBool((baseKey + ".es").c_str(), false);

        p.mode = _preferences.getString((baseKey + ".m").c_str(), "auto").c_str();
        config.boiler.periods.push_back(p);
    }

    _preferences.end();
    return config;
}

// Efface la configuration de la mémoire flash
void ConfigManager::clearConfig()
{
    _preferences.begin("config", false);
    _preferences.clear();
    _preferences.end();
}

void ConfigManager::printConfig(const Config &config)
{
    Serial.println("--- Configuration ---");
    Serial.println("Wifi:");
    Serial.print("  SSID: ");
    Serial.println(config.wifi.ssid.c_str());
    Serial.print("  Password: ");
    Serial.println(config.wifi.password.c_str()); // Be careful with sensitive info

    Serial.println("MQTT:");
    Serial.print("  Server: ");
    Serial.println(config.mqtt.server.c_str());
    Serial.print("  Port: ");
    Serial.println(config.mqtt.port);
    Serial.print("  Username: ");
    Serial.println(config.mqtt.username.c_str());
    Serial.print("  Password: ");
    Serial.println(config.mqtt.password.c_str()); // Be careful with sensitive info
    Serial.print("  Topic: ");
    Serial.println(config.mqtt.topic.c_str());

    Serial.println("Shelly EM:");
    Serial.print("  IP: ");
    Serial.println(config.shellyEm.ip.c_str());
    Serial.print("  Channel: ");
    Serial.println(config.shellyEm.channel.c_str());

    Serial.println("Boiler:");
    Serial.print("  Mode: ");
    Serial.println(config.boiler.mode.c_str());
    Serial.print("  Temperature: ");
    Serial.println(config.boiler.temperature);
    Serial.println("  Periods:");
    for (size_t i = 0; i < config.boiler.periods.size(); ++i)
    {
        const Period &p = config.boiler.periods[i];
        Serial.print("    Period ");
        Serial.print(i);
        Serial.print(": ");

        Serial.print("Start: ");
        if (p.startSunrise)
            Serial.print(" (Sunrise)");
        else if (p.startSunset)
            Serial.print(" (Sunset)");
        else
            Serial.print(p.start);

        Serial.print(", End: ");
        if (p.endSunrise)
            Serial.print(" (Sunrise)");
        else if (p.endSunset)
            Serial.print(" (Sunset)");
        else
            Serial.print(p.end);

        Serial.print(", Mode: ");
        Serial.println(p.mode.c_str());
    }

    Serial.println("Solar:");
    Serial.print("  Latitude: ");
    Serial.println(config.solar.latitude);
    Serial.print("  Longitude: ");
    Serial.println(config.solar.longitude);
    Serial.print("  TimeZone: ");
    Serial.println(config.solar.timeZone.c_str());
    Serial.print("  Sunrise Minutes: ");
    Serial.println(config.solar.sunRiseMinutes);
    Serial.print("  Sunset Minutes: ");
    Serial.println(config.solar.sunSetMinutes);
    Serial.println("---------------------");
}

std::string ConfigManager::getTriacMode(int sunRiseMinutes, int sunSetMinutes, const Config &config)
{

    time_t now;

    struct tm *timeinfo;

    time(&now);

    timeinfo = localtime(&now);

    int currentTimeInMinutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;

    BoilerConfig boilerConfig = config.boiler;

    // Check for "ON" periods first
    for (const auto &period : boilerConfig.periods)
    {
        if (period.mode == "on" || period.mode == "ON")
        {
            int startTime = 0;
            if (period.startSunrise)
            {
                startTime = sunRiseMinutes;
            }
            else if (period.startSunset)
            {
                startTime = sunSetMinutes;
            }
            else
            {
                startTime = period.start;
            }

            int endTime = 0;
            if (period.endSunrise)
            {
                endTime = sunRiseMinutes;
            }
            else if (period.endSunset)
            {
                endTime = sunSetMinutes;
            }
            else
            {
                endTime = period.end;
            }

            // Handle periods that cross midnight
            if (startTime > endTime)
            {
                if (currentTimeInMinutes >= startTime || currentTimeInMinutes < endTime)
                {
                    return "ON";
                }
            }
            else
            {
                if (currentTimeInMinutes >= startTime && currentTimeInMinutes < endTime)
                {
                    return "ON";
                }
            }
        }
    }

    // Then check for "AUTO" periods
    for (const auto &period : boilerConfig.periods)
    {
        if (period.mode == "auto" || period.mode == "AUTO")
        {
            int startTime = 0;
            if (period.startSunrise)
            {
                startTime = sunRiseMinutes;
            }
            else if (period.startSunset)
            {
                startTime = sunSetMinutes;
            }
            else
            {
                startTime = period.start;
            }

            int endTime = 0;
            if (period.endSunrise)
            {
                endTime = sunRiseMinutes;
            }
            else if (period.endSunset)
            {
                endTime = sunSetMinutes;
            }
            else
            {
                endTime = period.end;
            }

            // Handle periods that cross midnight
            if (startTime > endTime)
            {
                if (currentTimeInMinutes >= startTime || currentTimeInMinutes < endTime)
                {
                    return "AUTO";
                }
            }
            else
            {
                if (currentTimeInMinutes >= startTime && currentTimeInMinutes < endTime)
                {
                    return "AUTO";
                }
            }
        }
    }

    // Default to "OFF"
    return "OFF";
}
