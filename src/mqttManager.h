#ifndef TEMPERATUREMQTT_H
#define TEMPERATUREMQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

class MqttManager
{
public:
    // Constructeur avec WiFiClient en paramètre
    MqttManager();

    void setup(const char *server, int port, const char *username, const char *password, const char *topic);
    // Méthodes de connexion et d'envoi
    void connect(int timeout = 5);
    void sendTemperature(float temperature);
    // Méthode pour que homeAssistant découvre l'ESP32
    void sendDiscovery();

    // Fonction de mise à jour pour maintenir la connexion MQTT
    void loop();
    bool isConnected();

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
};

#endif