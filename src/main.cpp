#include <Arduino.h>
#include "WifiManager.h"
#include "MqttManager.h"
#include "files.h"
#include "webserverManager.h"
#include "sensor.h"
#include "configManager.h"
#include "shellyEm.h"
#include "SolarManager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>
#include "timezone.h"

// Pin Definitions
#define pinLedYellow 18
#define pinLedGreen 19
#define pinPulseTriac 22
#define pinZeroCross 23
#define pinTemperature 4

// Task Handles
TaskHandle_t CommunicationTaskHandle;
TaskHandle_t SignalProcessingTaskHandle;

// Global Objects
ConfigManager configManager;
Config config;
WifiManager wifiManager;
SolarManager *solarManager = nullptr;
MqttManager mqttManager(configManager);
WebServerManager web(configManager, mqttManager);
ShellyEm *shelly = nullptr;

// Shared Data
volatile float lastTemperature = 0;        // Dernière température mesurée
volatile float triacOpeningPercentage = 0; // Pourcentage d'ouverture du triac
volatile float lastPower = 0;              // Dernière puissance mesurée
int nowMinutes = 0;                        // Heure en minute
int sunriseMinutes = 0;                    // Heure de lever du soleil en minute
int sunsetMinutes = 0;                     // Heure du coucherdu soleil en minute
volatile bool temperatureReached = false;  // True si la température a été atteinte dans la journée (Remise à zéro au lever du soleil)
volatile bool reboot = false;              // true si on demande à l'ESP32 un reboot

// Mutex for thread-safe operations
SemaphoreHandle_t configMutex;

// Task for Signal Processing (Core 0)
void signalProcessingTask(void *pvParameters)
{
    Serial.println("Signal Processing Task started on core 0");
    static unsigned long lastShellyTime = 0;

    for (;;)
    {
        unsigned long now = millis();

        xSemaphoreTake(configMutex, portMAX_DELAY);
        std::string mode = config.boiler.mode;
        int boilerTemperature = config.boiler.temperature;
        xSemaphoreGive(configMutex);

        struct tm localNow;
        if (getLocalTime(&localNow))
        {

            struct tm sunrise = solarManager->calculateSunrise(config.solar.latitude, config.solar.longitude);
            struct tm sunset = solarManager->calculateSunset(config.solar.latitude, config.solar.longitude);

            nowMinutes = localNow.tm_hour * 60 + localNow.tm_min;
            sunriseMinutes = sunrise.tm_hour * 60 + sunrise.tm_min;
            sunsetMinutes = sunset.tm_hour * 60 + sunset.tm_min;

            // Obtient le mode lié à la configuration personnalisé des périodes
            std::string periodMode = configManager.getTriacMode(sunriseMinutes, sunsetMinutes, config);

            if (nowMinutes == sunriseMinutes)
            {
                // Remise à zero de température atteinte, avant le lever du soleil
                temperatureReached = false;
            }

            if (lastTemperature > config.boiler.temperature || temperatureReached)
            {
                // Le chauffe eau est chaud, plus besoin de régulation
                // Même si la température redescent en dessous de la température de consigne,
                // on ne relance le chauffe eau qu'après le prochain lever de soleil
                solarManager->Off();
                triacOpeningPercentage = 0;
                temperatureReached = true;
            }
            else if ((mode == "Auto" || mode == "auto") && periodMode == "AUTO")
            {
                // Toutes les 1 seconde, lecture de la puissance consommée
                if (shelly != nullptr && (now - lastShellyTime) > 1000)
                {

                    lastPower = shelly->getPower();
                    Serial.print("[ShellyEM] Puissance: ");
                    Serial.println(lastPower);
                    triacOpeningPercentage = solarManager->updateRegulation(lastPower);

                    // keep throttle timing regardless of whether we read or not
                    lastShellyTime = now;
                }
            }
            else if ((mode == "On" || mode == "on") || periodMode == "ON")
            {
                // Mode "Marche Forcé"
                solarManager->On();
                triacOpeningPercentage = 100;
            }
            else if ((mode == "Off" || mode == "off") || periodMode == "OFF")
            {
                // Mode "Arret Forcé"
                solarManager->Off();
                triacOpeningPercentage = 0;
            }
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
    static unsigned long lastMqttTime = 0;
    static unsigned long lastBroadCastweb = 0;

    for (;;)
    {
        unsigned long now = millis();

        xSemaphoreTake(configMutex, portMAX_DELAY);
        std::string mqttServer = config.mqtt.server;
        xSemaphoreGive(configMutex);

        if (reboot)
        {
            reboot = false;
            delay(3000);
            ESP.restart();
        }

        // Lecture de la température toutes les 30 secondes
        if (now - lastTempTime > 30 * 1000)
        {
            lastTemperature = getTemperature();
            lastTempTime = now;
        }
        // Brocast des donnée vers l'app web
        if (now - lastBroadCastweb > 1000)
        {
            web.broadcastData(lastTemperature, triacOpeningPercentage, temperatureReached);
            lastBroadCastweb = now;
        }

        if (!mqttServer.empty())
        {
            if (!mqttManager.isConnected())
            {
                mqttManager.connect(1);
            }
            mqttManager.loop();

            // Envoi du SendDiscovery à Home Assistant toutes les 5 minutes
            if (now - lastDiscoveryTime > 5 * 60 * 1000)
            {
                mqttManager.sendDiscovery();
                lastDiscoveryTime = now;
            }
            // Envoi des données à home assistant toutes les 10 secondes
            if (now - lastMqttTime > 10 * 1000)
            {
                mqttManager.sendData(lastTemperature, triacOpeningPercentage);
                lastMqttTime = now;
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
    reboot = false;

    // Create Mutex
    configMutex = xSemaphoreCreateMutex();

    // Pin initialization
    pinMode(pinLedYellow, OUTPUT);
    pinMode(pinLedGreen, OUTPUT);
    digitalWrite(pinLedYellow, LOW);
    digitalWrite(pinLedGreen, LOW);

    // Load configuration
    config = configManager.loadConfig();

    // Print Config
    configManager.printConfig(config);

    // Setup WiFi
    wifiManager.setupAccessPoint("ESP32_WROOM_SOLAR_ROUTER");
    wifiManager.connect(config.wifi.ssid.c_str(), config.wifi.password.c_str(), 5);

    // Setup LittleFS
    setupSpiffs();

    // Setup Web Server
    web.startServer();

    // Setup Sensor
    setupSensor();

    // Setup Shelly
    if (config.shellyEm.ip != "")
    {
        shelly = new ShellyEm(String(config.shellyEm.ip.c_str()), String(config.shellyEm.channel.c_str()));
    }

    // Setup Solar Manager
    solarManager = new SolarManager(pinPulseTriac, pinZeroCross);
    solarManager->begin();

    // Synchronize time with NTP server for Paris timezone
    Serial.println("[-] Synchronisation Date/Heure NTP server time.google.com");
    configTzTime(getPosixTimezone(config.solar.timeZone.c_str()), "time.google.com");

    // Récupère la date et l'heure locale
    struct tm timeinfo_local;
    if (getLocalTime(&timeinfo_local))
    {
        char timeStr[50];
        strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo_local);
        Serial.print("    - Date/Heure locale: ");
        Serial.println(timeStr);

        // // Calcul lever et coucher du soleil
        // struct tm sunrise = solarManager->calculateSunrise(config.solar.latitude, config.solar.longitude);
        // struct tm sunset = solarManager->calculateSunset(config.solar.latitude, config.solar.longitude);

        // if (sunrise.tm_year != -1)
        // {
        //     strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &sunrise);
        //     Serial.print("    - Lever du soleil: ");
        //     Serial.println(timeStr);
        // }
        // else
        // {
        //     Serial.println("    - Lever du soleil: (erreur)");
        // }

        // if (sunset.tm_year != -1)
        // {
        //     strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &sunset);
        //     Serial.print("    - Coucher du soleil: ");
        //     Serial.println(timeStr);
        // }
        // else
        // {
        //     Serial.println("    - Coucher du soleil: (erreur)");
        // }
    }

    // Setup MQTT
    if (config.mqtt.server != "")
    {
        mqttManager.setup(config.mqtt.server.c_str(), config.mqtt.port, config.mqtt.username.c_str(), config.mqtt.password.c_str(), config.mqtt.topic.c_str());
    }

    // Interrupts (should be safe, they are short)
    attachInterrupt(digitalPinToInterrupt(pinZeroCross), SolarManager::onZeroCrossStatic, RISING);

    hw_timer_t *timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &SolarManager::onTimerStatic, true);
    timerAlarmWrite(timer, 100, true);
    timerAlarmEnable(timer);

    // Create tasks
    xTaskCreatePinnedToCore(
        signalProcessingTask,        // Task function
        "SignalProcessingTask",      // Name of the task
        10000,                       // Stack size of task
        NULL,                        // Parameter of the task
        1,                           // Priority of the task
        &SignalProcessingTaskHandle, // Task handle to keep track of created task
        0);                          // Pin task to core 0

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
