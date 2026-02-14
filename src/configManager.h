#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Preferences.h>
#include <string>
#include <vector>

// Structure pour une période de fonctionnement
struct Period
{
    int start;
    bool startSunrise; // La période commence au lever du soleil
    bool startSunset;  // La période commence au coucher du soleil
    int end;
    bool endSunrise; // La période se termine au lever du soleil
    bool endSunset;  // La période se termine au coucher du soleil
    std::string mode;
};

// Structure pour la configuration WiFi
struct WifiConfig
{
    std::string ssid;
    std::string password;
};

// Structure pour la configuration MQTT
struct MqttConfig
{
    std::string server;
    int port;
    std::string username;
    std::string password;
    std::string topic;
};

// Structure pour la configuration Shelly EM
struct ShellyEmConfig
{
    std::string ip;
    std::string channel;
};

// Structure pour la configuration du chauffe-eau
struct BoilerConfig
{
    std::string mode;
    int temperature;
    std::vector<Period> periods;
    int triacOpening; // Pourcentage d'ouverture du triac en mode manuel (0-100)
};

// Structure pour la configuration solaire
struct SolarConfig
{
    float latitude;
    float longitude;
    int sunRiseMinutes;
    int sunSetMinutes;
    std::string timeZone;
};

// Structure principale de configuration
struct Config
{
    WifiConfig wifi;
    MqttConfig mqtt;
    ShellyEmConfig shellyEm;
    BoilerConfig boiler;
    SolarConfig solar;
};

class ConfigManager
{
public:
    ConfigManager();

    // Méthodes pour gérer la configuration
    bool saveConfig(const Config &config); // Sauvegarde la configuration dans la mémoire flash
    Config loadConfig();                   // Charge la configuration depuis la mémoire flash
    void clearConfig();                    // Efface la configuration de la mémoire flash
    std::string getTriacMode(int sunRiseMinutes, int sunSetMinutes, const Config &config);
    void printConfig(const Config &config);

private:
    Preferences _preferences;
};

#endif