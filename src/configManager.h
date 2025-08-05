#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Preferences.h>
#include <string>

// Structure pour stocker la configuration des paramètres WiFi, MQTT et des topics
struct Config
{
    std::string wifiSSID;        // SSID du réseau WiFi
    std::string wifiPassword;    // Mot de passe du réseau WiFi
    std::string mqttServer;      // Adresse du broker MQTT
    int mqttPort;                // Port du broker MQTT (par défaut 1883)
    std::string mqttUsername;    // Nom d'utilisateur du broker MQTT
    std::string mqttPassword;    // Mot de passe du broker MQTT
    std::string mqttTopic;       // Topic MQTT pour les données de température
    std::string shellyEmIp;      // Adresse Ip du module Shelly EM
    std::string shellyEmChannel; // Channel du module Shelly EM
    std::string boilerMode;      // Auto, On ou Off ( Auto = Routeur Solaire, On = Chauffe eau en marche forcé , Off = Chauffe à l'arret )
    int boilerPower;             // Puissance du chauffe-eau en watts
};

class ConfigManager
{
public:
    ConfigManager();

    // Méthodes pour gérer la configuration
    bool saveConfig(const Config &config); // Sauvegarde la configuration dans la mémoire flash
    Config loadConfig();                   // Charge la configuration depuis la mémoire flash
    void clearConfig();                    // Efface la configuration de la mémoire flash

private:
    Preferences _preferences;
};

#endif
