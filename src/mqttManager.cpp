#include "mqttManager.h"
#include <ArduinoJson.h>

// Constructeur de la classe
MqttManager::MqttManager() : client(espClient) {}

const std::string configTopic = "homeassistant/sensor/esp32_temperature/config";

void MqttManager::setup(const char *server, int port, const char *username, const char *password, const char *topic)
{
    this->server = server;
    this->port = port;
    this->username = username;
    this->password = password;
    this->topic = topic;

    client.setServer(server, port);
    client.setBufferSize(1024); // Augmente la taille du buffer  (256 par défaut = insuffisant )
}

// Méthode pour connecter au broker MQTT
void MqttManager::connect(int timeout)
{
    int i = 0;
    while (!client.connected() && i < timeout)
    {
        Serial.println("Connexion au broker MQTT... ");

        if (client.connect("ESP32Client", username.c_str(), password.c_str()))
        {
            Serial.println("Connecté au broker MQTT");
        }
        else
        {
            Serial.print("Échec, code d’erreur = ");
            Serial.print(client.state());
            Serial.println(" Nouvelle tentative ...");
            i++;
            delay(2000);
        }
    }

    if (i >= timeout)
    {
        Serial.println("  - Echec de connexion au broker MQTT");
    }
}

// Méthode pour envoyer la température au broker MQTT
void MqttManager::sendTemperature(float temperature)
{
    if (temperature != -127)
    {
        char msg[50];
        snprintf(msg, 50, "{\"temperature\":\"%.2f\"}", temperature);

        if (client.publish((topic + "/state").c_str(), msg))
        {
            Serial.print("[-] Send MQTT message :");
            Serial.println(msg);
        }
        else
        {
            Serial.println("    - MQTT : Échec de l'envoi des données");
        }
    }
}

// Méthode pour maintenir la connexion MQTT
void MqttManager::loop()
{
    client.loop();
}

bool MqttManager::isConnected()
{
    return client.connected();
}

void MqttManager::sendDiscovery()
{
    // Créer un message JSON pour le discovery de Home Assistant
    JsonDocument doc;
    doc["name"] = "ESP32 Temperature Sensor";
    doc["state_topic"] = topic + "/state";
    doc["unit_of_measurement"] = "°C";
    doc["device_class"] = "temperature";
    doc["unique_id"] = "esp32_temperature";
    doc["value_template"] = "{{ value_json.temperature}}";
    doc["icon"] = "mdi:thermometer";
    doc["device"]["name"] = "ESP32 Temperature Device";
    doc["device"]["identifiers"] = "esp32_temperature_device";
    doc["device"]["model"] = "ESP32";
    doc["device"]["manufacturer"] = "Demo ESP32 Compagny";

    // Convertir en chaîne JSON
    String jsonString;
    serializeJson(doc, jsonString);

    // Publier le message de configuration dans le topic de discovery de Home Assistant
    if (client.publish(configTopic.c_str(), jsonString.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassitant");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery.");
    }
}
