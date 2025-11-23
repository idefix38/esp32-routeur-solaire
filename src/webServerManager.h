#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "sensor.h"
#include "configManager.h"
#include "mqttManager.h"
#include "solarManager.h"
#include "updateManager.h"
#include "historyManager.h"

using namespace ArduinoJson;

class WebServerManager
{
public:
    WebServerManager(ConfigManager &configManager, MqttManager &mqttManager, HistoryManager &tempHistory, HistoryManager &triacHist);
    void setupLocalWeb();
    void setupApiRoutes();
    void startServer();
    void broadcastData(float temperature, float triacOpeningPercentage, bool temperatureReached, String newVersion = "");

private:
    void addFileRoutes(File dir);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handleReboot(AsyncWebServerRequest *request);
    void handleGetTemperatureHistory(AsyncWebServerRequest *request);
    void handleGetTriacHistory(AsyncWebServerRequest *request);
    void addCorsHeaders(AsyncWebServerResponse *response);
    void handleSaveWifiSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveMqttSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveSolarSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveBoilerSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    String getContentType(String filename);

    ConfigManager &configManager;
    MqttManager &mqttManager;
    HistoryManager &temperatureHistory;
    HistoryManager &triacHistory;
    UpdateManager updateManager;
    AsyncWebServer server;
    AsyncWebSocket ws;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    // Variables to track data changes for WebSocket broadcasting
    String lastBroadcastedJson;
    bool newClientConnected;
};

#endif