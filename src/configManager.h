#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Preferences.h>
#include <string>

// Structure pour stocker la configuration des paramètres WiFi, MQTT et des topics
struct ConfigTemperature
{
    std::string wifiSSID;     // SSID du réseau WiFi
    std::string wifiPassword; // Mot de passe du réseau WiFi
    std::string mqttServer;   // Adresse du broker MQTT
    int mqttPort;             // Port du broker MQTT (par défaut 1883)
    std::string mqttUsername; // Nom d'utilisateur du broker MQTT
    std::string mqttPassword; // Mot de passe du broker MQTT
    std::string mqttTopic;    // Topic MQTT pour les données de température
};

class ConfigManager
{
public:
    ConfigManager();

    // Méthodes pour gérer la configuration
    bool saveConfig(const ConfigTemperature &config); // Sauvegarde la configuration dans la mémoire flash
    ConfigTemperature loadConfig();                   // Charge la configuration depuis la mémoire flash
    void clearConfig();                               // Efface la configuration de la mémoire flash

private:
    Preferences _preferences;
};

#endif
