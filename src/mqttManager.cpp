#include "mqttManager.h"
#include <ArduinoJson.h>
#include "configManager.h"

// Le mode du chauffe-eau est maintenant un membre de la classe MqttManager

// Constructeur de la classe
MqttManager::MqttManager(ConfigManager &configManager) : configManager(configManager), client(espClient)
{
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
            // S'abonner aux topics de commande
            String modeCommandTopic = String(topic.c_str()) + "/boiler/mode/set";
            client.subscribe(modeCommandTopic.c_str());
            Serial.print("Abonnement au topic : ");
            Serial.println(modeCommandTopic);

            String tempCommandTopic = String(topic.c_str()) + "/boiler/temperature/set";
            client.subscribe(tempCommandTopic.c_str());
            Serial.print("Abonnement au topic : ");
            Serial.println(tempCommandTopic);

            // Publier l'état initial
            Config config = configManager.loadConfig();
            publishBoilerMode(config.boiler.mode.c_str());
            publishBoilerTemperature(config.boiler.temperature);
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
    String modeTopic = String(this->topic.c_str()) + "/boiler/mode/set";
    String tempTopic = String(this->topic.c_str()) + "/boiler/temperature/set";

    // Convertir le payload en String
    String payloadStr;
    for (unsigned int i = 0; i < length; i++)
    {
        payloadStr += (char)payload[i];
    }
    payloadStr.trim();

    if (topicStr == modeTopic)
    {
        if (payloadStr == "auto" || payloadStr == "on" || payloadStr == "off")
        {
            Config configTmp = this->configManager.loadConfig();
            configTmp.boiler.mode = payloadStr.c_str();
            this->configManager.saveConfig(configTmp);

            // Mise à jour avec la nouvelle config
            extern Config config;
            config = this->configManager.loadConfig();

            Serial.print("[MQTT] Mode chauffe-eau mis à jour : ");
            Serial.println(payloadStr);
            this->publishBoilerMode(payloadStr);
        }
        else
        {
            Serial.print("[MQTT] Mode reçu invalide : ");
            Serial.println(payloadStr);
        }
    }
    else if (topicStr == tempTopic)
    {
        int temp = payloadStr.toInt();
        if (temp >= 0 && temp <= 80) // Validation de la plage de température
        {
            Config configTmp = this->configManager.loadConfig();
            configTmp.boiler.temperature = temp;
            this->configManager.saveConfig(configTmp);

            extern Config config;
            config = configTmp;

            Serial.print("[MQTT] Température de consigne mise à jour : ");
            Serial.println(temp);
            this->publishBoilerTemperature(temp);
        }
        else
        {
            Serial.print("[MQTT] Température reçue invalide : ");
            Serial.println(payloadStr);
        }
    }
}

// Publier l'état du mode du chauffe-eau
void MqttManager::publishBoilerMode(String mode)
{
    if (mode != nullptr)
    {
        // Publie l'état du mode du chauffe-eau sur le topic approprié
        Serial.print("[MQTT] Publication de l'état du mode du chauffe-eau : ");
        Serial.println(mode);

        // Le topic pour l'état du mode du chauffe-eau

        String topic = String(this->topic.c_str()) + "/boiler/mode/state";
        client.publish(topic.c_str(), mode.c_str(), true);
    }
}

// Publier l'état de la température de consigne
void MqttManager::publishBoilerTemperature(int temperature)
{
    String topic = String(this->topic.c_str()) + "/boiler/temperature/state";
    client.publish(topic.c_str(), String(temperature).c_str(), true);
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

    // Découverte du capteur de pourcentage d'ouverture du triac
    String triacConfigTopic = "homeassistant/sensor/boiler/triac_opening/config";
    JsonDocument docTriac;
    docTriac["name"] = "Ouverture du Triac";
    docTriac["state_topic"] = topic + "/state";
    docTriac["unit_of_measurement"] = "%";
    docTriac["unique_id"] = "boiler_triac_opening";
    docTriac["value_template"] = "{{ value_json.triac_opening_percentage}}";
    docTriac["icon"] = "mdi:percent";
    docTriac["device"]["name"] = "Routeur solaire";
    docTriac["device"]["identifiers"] = "Routeur_solaire";
    docTriac["device"]["model"] = "ESP32";
    docTriac["device"]["manufacturer"] = "Mon routeur solaire";

    String jsonTriacString;
    serializeJson(docTriac, jsonTriacString);
    if (client.publish(triacConfigTopic.c_str(), jsonTriacString.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassitant (triac opening)");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery (triac opening).");
    }

    // Découverte du switch pour le mode du chauffe-eau
    String switchConfigTopic = "homeassistant/select/boiler/mode/config";
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

    // Découverte du number input pour la température du chauffe-eau
    String numberConfigTopic = "homeassistant/number/boiler/temperature_setpoint/config";
    JsonDocument docNumber;
    docNumber["name"] = "Consigne Température Chauffe-eau";
    docNumber["command_topic"] = topic + "/boiler/temperature/set";
    docNumber["state_topic"] = topic + "/boiler/temperature/state";
    docNumber["unique_id"] = "esp32_boiler_temp_setpoint";
    docNumber["unit_of_measurement"] = "°C";
    docNumber["device_class"] = "temperature";
    docNumber["min"] = 0;
    docNumber["max"] = 80;
    docNumber["step"] = 1;
    docNumber["icon"] = "mdi:thermometer-plus";
    docNumber["device"]["name"] = "Routeur solaire";
    docNumber["device"]["identifiers"] = "Routeur_solaire";
    docNumber["device"]["model"] = "ESP32";
    docNumber["device"]["manufacturer"] = "Mon routeur solaire";

    String jsonNumber;
    serializeJson(docNumber, jsonNumber);
    if (client.publish(numberConfigTopic.c_str(), jsonNumber.c_str(), true))
    {
        Serial.println("[-] Send discovery message to homeassistant (boiler temp setpoint)");
    }
    else
    {
        Serial.println("    - Échec de l'envoi du message discovery (boiler temp setpoint).");
    }
}

void MqttManager::sendData(float temperature, float triacOpeningPercentage)
{
    JsonDocument doc;
    // Arrondir les valeurs à deux décimales
    doc["temperature"] = round(temperature * 100) / 100.0;
    doc["triac_opening_percentage"] = round(triacOpeningPercentage * 100) / 100.0;

    String payload;
    serializeJson(doc, payload);

    String stateTopic = String(topic.c_str()) + "/state";

    // Log pour le débogage
    Serial.print("[MQTT] Payload: ");
    Serial.println(payload);

    if (!client.publish(stateTopic.c_str(), payload.c_str()))
    {
        Serial.println("    - Échec de l'envoi des données MQTT.");
    }
}
