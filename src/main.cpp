#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"

#include "TemSpiffs.h"
#include "webserverManager.h"
#include "sensor.h"
#include "configManager.h"
#include "shellyEm.h"

#define pinLedYellow 18
#define pinLedGreen 19
#define pinPulseTriac 22  // Activation du triac
#define pinZeroCross 23   // passage à zéro de la sinusoide du secteur
#define pinTemperature 13 // Capteur température

hw_timer_t *timer = NULL;

ConfigManager configManager;
Config config;

WifiManager wifiManager;
MqttManager mqttManager = MqttManager(configManager);
WebServerManager web = WebServerManager(configManager, mqttManager);
ShellyEm *shelly = nullptr; // Pointeur pour l'objet ShellyEm

#include "SolarManager.h"
SolarManager *solarManager = nullptr; // Déclaré comme un pointeur

void setup()
{
    Serial.begin(115200);

    // Pin initialisation
    pinMode(pinLedYellow, OUTPUT);
    pinMode(pinLedGreen, OUTPUT);
    digitalWrite(pinLedYellow, LOW);
    digitalWrite(pinLedGreen, LOW);

    // ---------------------------Read Config --------------------
    config = configManager.loadConfig(); // La configuration est chargée ici

    // --- Log config as JSON ---
    JsonDocument doc;
    doc["wifiSSID"] = config.wifiSSID;
    // doc["wifiPassword"] = config.wifiPassword; // For security, don't log password
    doc["mqttServer"] = config.mqttServer;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUsername"] = config.mqttUsername;
    // doc["mqttPassword"] = config.mqttPassword; // For security, don't log password
    doc["mqttTopic"] = config.mqttTopic;
    doc["shellyEmIp"] = config.shellyEmIp;
    doc["shellyEmChannel"] = config.shellyEmChannel;
    doc["boilerMode"] = config.boilerMode;
    doc["boilerPower"] = config.boilerPower;
    String jsonConfig;
    serializeJsonPretty(doc, jsonConfig);
    Serial.println("--------- Configuration chargée ---------");
    Serial.println(jsonConfig);
    Serial.println("------------------------------------");

    // ---------------------------- Setup Shelly ----------------------
    // Initialisation de Shelly après le chargement de la config
    if (config.shellyEmIp != "")
    {
        shelly = new ShellyEm(String(config.shellyEmIp.c_str()), String(config.shellyEmChannel.c_str()));
    }

    // ---------------------------- Setup Solar Manager -----------------
    // Initialisation après le chargement de la config
    solarManager = new SolarManager(pinPulseTriac, pinZeroCross, config.boilerPower);
    solarManager->begin();

    // ---------------------------- Setup du Wifi ---------------------
    wifiManager.setupAccessPoint("ESP32_WROOM_SOLAR_ROUTER");
    wifiManager.connect(config.wifiSSID.c_str(), config.wifiPassword.c_str(), 5);
    //---------------------------- SPIFFS ------------------------------
    setupSpiffs();
    //---------------------------- SERVEUR WEB ---------------------------
    web.startServer();
    // ---------------------------- GPIO -------------------------------
    setupSensor();
    // //----------------------------- Setup Mqtt -------------------------
    if (config.mqttServer != "")
    {
        mqttManager.setup(config.mqttServer.c_str(), config.mqttPort, config.mqttUsername.c_str(), config.mqttPassword.c_str(), config.mqttTopic.c_str());
        mqttManager.connect(1);
        mqttManager.sendDiscovery();
    }

    // Interruptions du Triac et Timer interne
    attachInterrupt(pinZeroCross, SolarManager::onZeroCrossStatic, RISING);

    // Hardware timer
    timer = timerBegin(0, 80, true); // Clock Divider, 1 micro second Tick
    timerAttachInterrupt(timer, &SolarManager::onTimerStatic, true);
    timerAlarmWrite(timer, 100, true); // Interrupt every 100 Ticks or microsecond
    timerAlarmEnable(timer);
}

void loop()
{
    static unsigned long lastDiscoveryTime = 0;
    static unsigned long lastTempTime = 0;
    static unsigned long lastShellyTime = 0;
    static float lastTemperature = 0;
    static float lastPower = 0;
    static float regulatedPower = 0;

    unsigned long now = millis();

    // Publication du message Discovery toutes les 5 minutes
    if (config.mqttServer != "" && now - lastDiscoveryTime > 5 * 60 * 1000)
    {
        if (!mqttManager.isConnected())
        {
            mqttManager.connect(1);
        }
        mqttManager.sendDiscovery();
        lastDiscoveryTime = now;
    }

    // Mesure et envoi des données toutes les 5 secondes
    if (now - lastTempTime > 5 * 1000)
    {
        lastTemperature = getTemperature();
        if (config.mqttServer != "")
        {
            // CORRECTION : On vérifie la connexion avant d'envoyer
            if (!mqttManager.isConnected())
            {
                mqttManager.connect(1);
            }
            mqttManager.sendData(lastTemperature, regulatedPower);
        }
        lastTempTime = now;
    }

    // Appel au ShellyEM toutes les secondes
    if ((config.boilerMode == "Auto" || config.boilerMode == "auto") && (now - lastShellyTime) > 1000)
    {
        // AMÉLIORATION : On utilise l'objet global et on vérifie qu'il existe
        if (shelly != nullptr)
        {
            lastPower = shelly->getPower();
            Serial.print("[ShellyEM] Puissance: ");
            Serial.println(lastPower);
            regulatedPower = solarManager->RegulationProduction(lastPower);
        }
        lastShellyTime = now;
    }
    if (config.boilerMode == "On" || config.boilerMode == "on")
    {
        solarManager->On();
        regulatedPower = config.boilerPower; // Utilise la puissance du chauffe-eau
    }
    if (config.boilerMode == "Off" || config.boilerMode == "off")
    {
        solarManager->Off();
        regulatedPower = 0; // Pas de puissance
    }

    mqttManager.loop();
}