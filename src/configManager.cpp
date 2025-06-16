#include "ConfigManager.h"

// Constructeur
ConfigManager::ConfigManager()
{
}

// Sauvegarde la configuration dans la mémoire flash
bool ConfigManager::saveConfig(const ConfigTemperature &config)
{

    Serial.println("[-] Ecriture de la mémoire flash ...");

    _preferences.begin("config", false); // "config" est l'espace de noms pour stocker les configurations
    _preferences.putString("wifiSSID", config.wifiSSID.c_str());
    _preferences.putString("wifiPassword", config.wifiPassword.c_str());
    _preferences.putString("mqttServer", config.mqttServer.c_str());
    _preferences.putInt("mqttPort", config.mqttPort);
    _preferences.putString("mqttUsername", config.mqttUsername.c_str());
    _preferences.putString("mqttPassword", config.mqttPassword.c_str());
    _preferences.putString("mqttTopic", config.mqttTopic.c_str());
    _preferences.end(); // Ferme l'accès aux préférences

    return true; // Retourne vrai si la sauvegarde est réussie
}

// Charge la configuration depuis la mémoire flash et retourne un objet ConfigTemperature
ConfigTemperature ConfigManager::loadConfig()
{
    ConfigTemperature config;

    Serial.println("[-] Lecture de la mémoire flash ...");
    _preferences.begin("config", false); // "config" est l'espace de noms pour stocker les configurations
    config.wifiSSID = _preferences.getString("wifiSSID", "").c_str();
    config.wifiPassword = _preferences.getString("wifiPassword", "").c_str();
    config.mqttServer = _preferences.getString("mqttServer", "").c_str();
    config.mqttPort = _preferences.getInt("mqttPort", 1883); // Port par défaut
    config.mqttUsername = _preferences.getString("mqttUsername", "").c_str();
    config.mqttPassword = _preferences.getString("mqttPassword", "").c_str();
    config.mqttTopic = _preferences.getString("mqttTopic", "").c_str();
    _preferences.end();

    return config; // Retourne l'objet de configuration
}

// Efface la configuration de la mémoire flash
void ConfigManager::clearConfig()
{
    _preferences.clear(); // Supprime toutes les clés de l'espace de noms "network"
}
