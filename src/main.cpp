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
WebServerManager web = WebServerManager(configManager);

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
            mqttManager.sendData(lastTemperature, regulatedPower);
        }
        lastTempTime = now;
    }

    // Appel au ShellyEM toutes les secondes
    if (config.boilerMode == "auto" && (now - lastShellyTime) > 1000)
    {
        ShellyEm shelly(String(config.shellyEmIp.c_str()), String(config.shellyEmChannel.c_str()));
        lastPower = shelly.getPower();
        Serial.print("[ShellyEM] Puissance: ");
        Serial.println(lastPower);
        regulatedPower = solarManager->RegulationProduction(lastPower);
        lastShellyTime = now;
    }
    if (config.boilerMode == "on")
    {
        solarManager->On();
    }
    if (config.boilerMode == "off")
    {
        solarManager->Off();
    }

    mqttManager.loop();
}