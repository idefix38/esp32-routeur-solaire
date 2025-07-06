#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"

#include "TemSpiffs.h"
#include "webserverManager.h"
#include "sensor.h"
#include "configManager.h"

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
    float temperature = getTemperature();
    static unsigned long lastDiscoveryTime = 0;

    if (config.mqttServer != "")
    {
        if (!mqttManager.isConnected())
        {
            mqttManager.connect(1);
        }
        // Republier le message discovery toutes les 5 minutes
        // Necessaire en cas de redemarrage de home assistant
        if (millis() - lastDiscoveryTime > 5 * 60 * 1000)
        { // 10 minutes
            mqttManager.sendDiscovery();
            lastDiscoveryTime = millis();
        }
        mqttManager.loop();
        mqttManager.sendTemperature(temperature);
    }
    sleep(10);
    Serial.println("[-] Loop ...");
}
