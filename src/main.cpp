#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"
#include "TemSpiffs.h"
#include "webserverManager.h"
#include "sensor.h"
#include "configManager.h"
#include "shellyEm.h"
#include "SolarManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Pin Definitions
#define pinLedYellow 18
#define pinLedGreen 19
#define pinPulseTriac 22
#define pinZeroCross 23
#define pinTemperature 13

// Task Handles
TaskHandle_t CommunicationTaskHandle;
TaskHandle_t SignalProcessingTaskHandle;

// Global Objects
ConfigManager configManager;
Config config;
WifiManager wifiManager;
MqttManager mqttManager(configManager);
WebServerManager web(configManager, mqttManager);
ShellyEm *shelly = nullptr;
SolarManager *solarManager = nullptr;

// Shared Data
volatile float lastTemperature = 0;
volatile float regulatedPower = 0;

// Mutex for thread-safe operations
SemaphoreHandle_t configMutex;

// Task for Signal Processing (Core 0)
void signalProcessingTask(void *pvParameters)
{
    Serial.println("Signal Processing Task started on core 0");
    static unsigned long lastShellyTime = 0;
    float lastPower = 0;

    for (;;)
    {
        unsigned long now = millis();

        xSemaphoreTake(configMutex, portMAX_DELAY);
        std::string mode = config.boilerMode;
        int boilerPower = config.boilerPower;
        xSemaphoreGive(configMutex);

        if (mode == "Auto" || mode == "auto")
        {
            if ((now - lastShellyTime) > 1000)
            {
                if (shelly != nullptr)
                {
                    lastPower = shelly->getPower();
                    Serial.print("[ShellyEM] Puissance: ");
                    Serial.println(lastPower);
                    regulatedPower = solarManager->RegulationProduction(lastPower);
                }
                lastShellyTime = now;
            }
        }
        else if (mode == "On" || mode == "on")
        {
            solarManager->On();
            regulatedPower = boilerPower;
        }
        else if (mode == "Off" || mode == "off")
        {
            solarManager->Off();
            regulatedPower = 0;
        }

        // Delay to prevent task from hogging the CPU
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Task for Communication (Core 1)
void communicationTask(void *pvParameters)
{
    Serial.println("Communication Task started on core 1");
    static unsigned long lastDiscoveryTime = 0;
    static unsigned long lastTempTime = 0;

    for (;;)
    {
        unsigned long now = millis();

        xSemaphoreTake(configMutex, portMAX_DELAY);
        std::string mqttServer = config.mqttServer;
        xSemaphoreGive(configMutex);

        if (!mqttServer.empty())
        {
            if (!mqttManager.isConnected())
            {
                mqttManager.connect(1);
            }
            mqttManager.loop();

            if (now - lastDiscoveryTime > 5 * 60 * 1000)
            {
                mqttManager.sendDiscovery();
                lastDiscoveryTime = now;
            }

            if (now - lastTempTime > 5000)
            {
                lastTemperature = getTemperature();
                mqttManager.sendData(lastTemperature, regulatedPower);
                lastTempTime = now;
            }
        }

        // The web server logic (especially if async) handles its own connections.
        // No explicit web.loop() is needed if using ESPAsyncWebServer.

        // Delay to yield to other tasks, if any, on the same core
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void setup()
{
    Serial.begin(115200);

    // Create Mutex
    configMutex = xSemaphoreCreateMutex();

    // Pin initialization
    pinMode(pinLedYellow, OUTPUT);
    pinMode(pinLedGreen, OUTPUT);
    digitalWrite(pinLedYellow, LOW);
    digitalWrite(pinLedGreen, LOW);

    // Load configuration
    config = configManager.loadConfig();

    // Log config
    JsonDocument doc;
    doc["wifiSSID"] = config.wifiSSID;
    doc["mqttServer"] = config.mqttServer;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUsername"] = config.mqttUsername;
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

    // Setup WiFi
    wifiManager.setupAccessPoint("ESP32_WROOM_SOLAR_ROUTER");
    wifiManager.connect(config.wifiSSID.c_str(), config.wifiPassword.c_str(), 5);

    // Setup SPIFFS
    setupSpiffs();

    // Setup Web Server
    web.startServer();

    // Setup Sensor
    setupSensor();

    // Setup Shelly
    if (config.shellyEmIp != "")
    {
        shelly = new ShellyEm(String(config.shellyEmIp.c_str()), String(config.shellyEmChannel.c_str()));
    }

    // Setup Solar Manager
    solarManager = new SolarManager(pinPulseTriac, pinZeroCross, config.boilerPower);
    solarManager->begin();

    // Setup MQTT
    if (config.mqttServer != "")
    {
        mqttManager.setup(config.mqttServer.c_str(), config.mqttPort, config.mqttUsername.c_str(), config.mqttPassword.c_str(), config.mqttTopic.c_str());
    }

    // Interrupts (should be safe, they are short)
    attachInterrupt(digitalPinToInterrupt(pinZeroCross), SolarManager::onZeroCrossStatic, RISING);
    
    hw_timer_t *timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &SolarManager::onTimerStatic, true);
    timerAlarmWrite(timer, 100, true);
    timerAlarmEnable(timer);

    // Create tasks
    xTaskCreatePinnedToCore(
        signalProcessingTask,      // Task function
        "SignalProcessingTask",    // Name of the task
        10000,                     // Stack size of task
        NULL,                      // Parameter of the task
        1,                         // Priority of the task
        &SignalProcessingTaskHandle, // Task handle to keep track of created task
        0);                        // Pin task to core 0

    xTaskCreatePinnedToCore(
        communicationTask,        // Task function
        "CommunicationTask",      // Name of the task
        10000,                    // Stack size of task
        NULL,                     // Parameter of the task
        1,                        // Priority of the task
        &CommunicationTaskHandle, // Task handle to keep track of created task
        1);                       // Pin task to core 1
}

void loop()
{
    // Empty. Everything is handled in tasks.
    vTaskDelete(NULL); // Delete the loop task
}
