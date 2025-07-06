#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "sensor.h"
#include "ConfigManager.h"

using namespace ArduinoJson;

class WebServerManager
{
public:
    WebServerManager(ConfigManager &configManager); // Removed explicit since it's a non-const reference
    void setupLocalWeb();
    void setupApiRoutes();
    void startServer();

private:
    void handleGetConfig(AsyncWebServerRequest *request);
    void addCorsHeaders(AsyncWebServerResponse *response);
    void handleSaveWifiSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveMqttSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    void handleSaveSolarSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len);
    String getContentType(String filename);

    ConfigManager &configManager;
    AsyncWebServer server;
};

#endif