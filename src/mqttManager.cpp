#include "mqttManager.h"
#include <ArduinoJson.h>
#include "configManager.h"

// Le mode du chauffe-eau est maintenant un membre de la classe MqttManager

// Constructeur de la classe
MqttManager::MqttManager(ConfigManager &configManager) : configManager(configManager), client(espClient), boilerMode("auto")
{
    // Le callback sera défini dans setup() (quand 'this' est bien initialisé)
}

const std::string configTopic = "homeassistant/sensor/boiler/config";

void MqttManager::setup(const char *server, int port, const char *username, const char *password, const char *topic)
{
    this->server = server;
    this->port = port;
    this->username = username;
    this->password = password;
    this->topic = topic;

    client.setServer(server, port);
    client.setBufferSize(1024); // Augmente la taille du buffer  (256 par défaut = insuffisant )

    // Définir le callback MQTT pour la réception de messages
    client.setCallback([this](char *topic, byte *payload, unsigned int length)
                       { this->onMqttMessage(topic, payload, length); });
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
            // S'abonner au topic de commande du chauffe-eau
            String commandTopic = String(topic.c_str()) + "/boiler/mode/set";
            client.subscribe(commandTopic.c_str());

            Serial.print("Abonnement au topic : ");
            Serial.println(commandTopic);
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

// Callback appelé lors de la réception d'un message MQTT
void MqttManager::onMqttMessage(char *topic, byte *payload, unsigned int length)
{
    String topicStr = String(topic);
    String expectedTopic = String(this->topic.c_str());
    expectedTopic += "/boiler/mode/set";
    if (topicStr == expectedTopic)
    {
        String modePayload;
        for (unsigned int i = 0; i < length; i++)
        {
            modePayload += (char)payload[i];
        }
        modePayload.trim();
        if (modePayload == "auto" || modePayload == "on" || modePayload == "off")
        {
            this->boilerMode = modePayload;
            Config config = this->configManager.loadConfig();
            config.boilerMode = this->boilerMode.c_str();
            this->configManager.saveConfig(config);
            Serial.print("[MQTT] Mode chauffe-eau mis à jour : ");
            Serial.println(this->boilerMode);
            // Publier le nouvel état sur le topic state
            this->publishBoilerMode();
        }
        else
        {
            Serial.print("[MQTT] Mode reçu invalide : ");
            Serial.println(modePayload);
        }
    }
}

// Publier l'état du mode du chauffe-eau
void MqttManager::publishBoilerMode()
{
    String stateTopic = String(this->topic.c_str()) + "/boiler/mode/state";
    client.publish(stateTopic.c_str(), boilerMode.c_str(), true);
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
    // Découverte du capteur de température
    JsonDocument doc;
    doc["name"] = "Température Chauffe-Eau";
    doc["state_topic"] = topic + "/state";
    doc["unit_of_measurement"] = "°C";
    doc["device_class"] = "temperature";
    doc["unique_id"] = "boiler_temperature";
    doc["value_template"] = "{{ value_json.temperature}}";
    doc["icon"] = "mdi:thermometer";
    doc["device"]["name"] = "Routeur solaire";
    doc["device"]["identifiers"] = "Routeur_solaire";
    doc["device"]["model"] = "ESP32";
    doc["device"]["manufacturer"] = "Mon routeur solaire";

    String jsonString;
    serializeJson(doc, jsonString);
    String tempConfigTopic = "homeassistant/sensor/boiler/temperature/config";
    if (client.publish(tempConfigTopic.c_str(), jsonString.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassitant (temp)");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery (temp).");
    }

    // Découverte du capteur de puissance
    String powerConfigTopic = "homeassistant/sensor/boiler/power/config";
    JsonDocument docPower;
    docPower["name"] = "Puissance envoyée au Chauffe-Eau";
    docPower["state_topic"] = topic + "/state";
    docPower["unit_of_measurement"] = "W";
    docPower["device_class"] = "power";
    docPower["unique_id"] = "boiler_power";
    docPower["value_template"] = "{{ value_json.power}}";
    docPower["icon"] = "mdi:flash";
    docPower["device"]["name"] = "Routeur solaire";
    docPower["device"]["identifiers"] = "Routeur_solaire";
    docPower["device"]["model"] = "ESP32";
    docPower["device"]["manufacturer"] = "Mon routeur solaire";

    String jsonPowerString;
    serializeJson(docPower, jsonPowerString);
    if (client.publish(powerConfigTopic.c_str(), jsonPowerString.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassitant (power)");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery (power).");
    }

    // Découverte du switch pour le mode du chauffe-eau
    String switchConfigTopic = "homeassistant/select/boiler_mode/config";
    JsonDocument docSwitch;
    docSwitch["name"] = "Mode du Chauffe eau";
    docSwitch["command_topic"] = topic + "/boiler/mode/set";
    docSwitch["state_topic"] = topic + "/boiler/mode/state";
    docSwitch["unique_id"] = "esp32_boiler_mode";
    JsonArray options = docSwitch["options"].to<JsonArray>();
    options.add("auto");
    options.add("on");
    options.add("off");
    docSwitch["icon"] = "mdi:water-boiler";
    docSwitch["device"]["name"] = "Routeur solaire";
    docSwitch["device"]["identifiers"] = "Routeur_solaire";
    docSwitch["device"]["model"] = "ESP32";
    docSwitch["device"]["manufacturer"] = "Mon routeur solaire";

    String jsonSwitch;
    serializeJson(docSwitch, jsonSwitch);
    if (client.publish(switchConfigTopic.c_str(), jsonSwitch.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassitant (boiler mode)");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery (boiler mode).");
    }
}

void MqttManager::sendData(float temperature, float power)
{
    JsonDocument doc;
    // Arrondir les valeurs à deux décimales
    doc["temperature"] = round(temperature * 100) / 100.0;
    doc["power"] = round(power * 100) / 100.0;

    String payload;
    serializeJson(doc, payload);

    String stateTopic = String(topic.c_str()) + "/state";
    if (!client.publish(stateTopic.c_str(), payload.c_str()))
    {
        Serial.println("    - Échec de l'envoi des données MQTT.");
    }
}
