#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "sensor.h"
#include "ConfigManager.h"
#include "mqttManager.h"

using namespace ArduinoJson;

class WebServerManager
{
public:
    WebServerManager(ConfigManager &configManager, MqttManager &mqttManager);
    void setupLocalWeb();
    void setupApiRoutes();
    void startServer();
    void broadcastData(float temperature, float triacOpeningPercentage, String sunrise, String sunset);

private:
    void addFileRoutes(File dir);
    void handleGetConfig(AsyncWebServerRequest *request);
    void handleReboot(AsyncWebServerRequest *request);
    void addCorsHeaders(AsyncWebServerResponse *response);
    void handleSaveWifiSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveMqttSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveSolarSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    String getContentType(String filename);

    ConfigManager &configManager;
    MqttManager &mqttManager;
    AsyncWebServer server;
    AsyncWebSocket ws;
    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

    // Variables to track data changes for WebSocket broadcasting
    String lastBroadcastedJson;
    bool newClientConnected;
};

#endif