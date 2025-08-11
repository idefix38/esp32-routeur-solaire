#ifndef TEMPERATUREMQTT_H
#define TEMPERATUREMQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "configManager.h"

class MqttManager
{
public:
    // Constructeur avec WiFiClient en paramètre
    MqttManager(ConfigManager &configManager);

    void setup(const char *server, int port, const char *username, const char *password, const char *topic);
    // Méthodes de connexion et d'envoi
    void connect(int timeout = 5);
    void sendData(float temperature, float power);
    // Méthode pour que homeAssistant découvre l'ESP32
    void sendDiscovery();

    // Fonction de mise à jour pour maintenir la connexion MQTT
    void loop();
    bool isConnected();
    // Publication de l'état du mode du chauffe-eau
    void publishBoilerMode(String boilerMode);

    // Ajout du getter pour boilerMode
    // String getBoilerMode() const { return boilerMode; }

private:
    // Variables de configuration WiFi et MQTT
    std::string server;
    int port;
    std::string username;
    std::string password;
    std::string topic;
    // Référence vers un client WiFi et un objet client MQTT
    WiFiClient espClient;
    PubSubClient client;
    ConfigManager &configManager;

    // Mode du chauffe-eau
    // String boilerMode;

    // Callback pour la réception de messages MQTT
    void onMqttMessage(char *topic, byte *payload, unsigned int length);
};

#endif