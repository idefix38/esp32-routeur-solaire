#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"

#include "TemSpiffs.h"
#include "webserverManager.h"
#include "sensor.h"
#include "configManager.h"
#include "shellyEm.h"

ConfigManager configManager;
Config config;

WifiManager wifiManager;
MqttManager mqttManager;
WebServerManager web = WebServerManager(configManager);

void setup()
{
    Serial.begin(115200);
    // ---------------------------Read Config --------------------
    config = configManager.loadConfig();
    // ---------------------------- Setup du Wifi ---------------------
    wifiManager.setupAccessPoint("ESP32_WROOM_TEMPERATURE");
    wifiManager.connect(config.wifiSSID.c_str(), config.wifiPassword.c_str(), 5);
    //---------------------------- SPIFFS ------------------------------
    setupSpiffs();
    //---------------------------- SERVEUR WEB ---------------------------
    web.startServer();
    // //---------------------------- GPIO -------------------------------
    setupSensor();
    // //----------------------------- Setup Mqtt -------------------------
    if (config.mqttServer != "")
    {
        mqttManager.setup(config.mqttServer.c_str(), config.mqttPort, config.mqttUsername.c_str(), config.mqttPassword.c_str(), config.mqttTopic.c_str());
        mqttManager.connect(1);
        mqttManager.sendDiscovery();
    }
}

void loop()
{
    static unsigned long lastDiscoveryTime = 0;
    static unsigned long lastTempTime = 0;
    static unsigned long lastShellyTime = 0;
    static float lastTemperature = 0;
    static float lastPower = 0;

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

    // Mesure et envoi de la température toutes les 30 secondes
    if (now - lastTempTime > 30 * 1000)
    {
        lastTemperature = getTemperature();
        if (config.mqttServer != "")
        {
            mqttManager.sendTemperature(lastTemperature);
        }
        lastTempTime = now;
    }

    // Appel au ShellyEM toutes les secondes
    if (now - lastShellyTime > 1000)
    {
        ShellyEm shelly(String(config.shellyEmIp.c_str()), String(config.shellyEmChannel.c_str()));
        lastPower = shelly.getPower();
        Serial.print("[ShellyEM] Puissance: ");
        Serial.println(lastPower);
        lastShellyTime = now;
    }

    mqttManager.loop();
}
