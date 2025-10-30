#include "WebServerManager.h"
#include "mqttManager.h"
#include "SolarManager.h"

// Constructeur
WebServerManager::WebServerManager(ConfigManager &configManager, MqttManager &mqttManager)
    : configManager(configManager), mqttManager(mqttManager), server(80), ws("/ws")
{
    lastBroadcastedJson = "";
    newClientConnected = false;
}

void WebServerManager::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf(" [-] WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        // Set flag to send data to the new client immediately
        this->newClientConnected = true;
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf(" [-] WebSocket client #%u disconnected\n", client->id());
    }
}

void WebServerManager::handleReboot(AsyncWebServerRequest *request)
{
    Serial.println(" GET: /reboot");
    AsyncWebServerResponse *response = request->beginResponse(200, "text/html", "ok");
    extern volatile bool reboot;
    reboot = true;
    request->send(response);
}

void WebServerManager::handleGetConfig(AsyncWebServerRequest *request)
{
    Serial.println(" GET: /getConfig");
    Config config = this->configManager.loadConfig();

    JsonDocument doc;
    JsonObject wifiObj = doc["wifi"].to<JsonObject>();
    wifiObj["ssid"] = config.wifi.ssid;
    wifiObj["password"] = "********";

    JsonObject mqttObj = doc["mqtt"].to<JsonObject>();
    mqttObj["server"] = config.mqtt.server;
    mqttObj["port"] = config.mqtt.port;
    mqttObj["username"] = config.mqtt.username;
    mqttObj["password"] = "********";
    mqttObj["topic"] = config.mqtt.topic;

    JsonObject shellyObj = doc["shellyEm"].to<JsonObject>();
    shellyObj["ip"] = config.shellyEm.ip;
    shellyObj["channel"] = config.shellyEm.channel;

    JsonObject boilerObj = doc["boiler"].to<JsonObject>();
    boilerObj["mode"] = config.boiler.mode;
    boilerObj["temperature"] = config.boiler.temperature;
    JsonArray boilerPeriods = boilerObj["periods"].to<JsonArray>();
    for (const auto &p : config.boiler.periods)
    {
        JsonObject periodObj = boilerPeriods.add<JsonObject>();
        periodObj["start"] = p.start;
        periodObj["end"] = p.end;
        periodObj["mode"] = p.mode;
    }

    JsonObject solarObj = doc["solar"].to<JsonObject>();
    solarObj["latitude"] = config.solar.latitude;
    solarObj["longitude"] = config.solar.longitude;
    solarObj["timeZone"] = config.solar.timeZone;
    solarObj["sunRiseMinutes"] = config.solar.sunRiseMinutes;
    solarObj["sunSetMinutes"] = config.solar.sunSetMinutes;

    String jsonString;
    serializeJson(doc, jsonString);
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
    request->send(response);
}

void WebServerManager::handleSaveWifiSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len)
{
    Serial.println(" POST: /saveWifiSettings");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error)
    {
        Serial.println("Erreur de parsing du JSON !");
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
    }

    Config config = this->configManager.loadConfig();
    config.wifi.ssid = doc["ssid"] | "";
    const char *password = doc["password"] | "";
    if (password != "" && strcmp(password, "********") != 0)
    {
        config.wifi.password = password;
    }
    this->configManager.saveConfig(config);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::handleSaveMqttSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len)
{
    Serial.println(" POST: /saveMqttSettings");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error)
    {
        Serial.println("Erreur de parsing du JSON !");
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
    }

    Config config = this->configManager.loadConfig();
    config.mqtt.server = doc["server"] | "";
    config.mqtt.port = doc["port"] | 1883;
    config.mqtt.topic = doc["topic"] | "";
    config.mqtt.username = doc["username"] | "";
    const char *password = doc["password"] | "";
    if (password != "" && strcmp(password, "********") != 0)
    {
        config.mqtt.password = password;
    }
    this->configManager.saveConfig(config);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::handleSaveSolarSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len)
{
    Serial.println(" POST: /saveSolarSettings");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error)
    {
        Serial.println("Erreur de parsing du JSON !");
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
    }

    Config configTmp = this->configManager.loadConfig();

    if (!doc["shellyEm"].isNull())
    {
        configTmp.shellyEm.ip = doc["shellyEm"]["ip"] | "";
        configTmp.shellyEm.channel = doc["shellyEm"]["channel"] | "0";
    }

    if (!doc["solar"].isNull())
    {
        configTmp.solar.latitude = doc["solar"]["latitude"] | 48.8566;
        configTmp.solar.longitude = doc["solar"]["longitude"] | 2.3522;
        configTmp.solar.timeZone = doc["solar"]["timeZone"] | "Europe/Paris";
    }

    this->configManager.saveConfig(configTmp);

    extern Config config;
    config = configTmp;

    this->mqttManager.publishBoilerMode(config.boiler.mode.c_str());
    this->mqttManager.publishBoilerTemperature(config.boiler.temperature);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::handleSaveBoilerSettings(AsyncWebServerRequest *request, uint8_t *data, size_t len)
{
    Serial.println(" POST: /saveBoilerSettings");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data, len);

    if (error)
    {
        Serial.println("Erreur de parsing du JSON !");
        request->send(400, "application/json", "{\"status\":\"Invalid JSON\"}");
        return;
    }

    Config configTmp = this->configManager.loadConfig();

    configTmp.boiler.mode = doc["mode"] | "auto";
    configTmp.boiler.temperature = doc["temperature"] | 50;
    if (!doc["periods"].isNull())
    {
        configTmp.boiler.periods.clear();
        for (JsonObject p : doc["periods"].as<JsonArray>())
        {
            Period period;
            period.start = p["start"];
            period.end = p["end"];
            period.mode = p["mode"] | "auto";
            period.startSunrise = p["start"] == configTmp.solar.sunRiseMinutes;
            period.startSunset = p["start"] == configTmp.solar.sunSetMinutes;
            period.endSunrise = p["end"] == configTmp.solar.sunRiseMinutes;
            period.endSunset = p["end"] == configTmp.solar.sunSetMinutes;
            configTmp.boiler.periods.push_back(period);
        }
    }

    this->configManager.saveConfig(configTmp);

    extern Config config;
    config = configTmp;
    extern bool temperatureReached;
    temperatureReached = false;

    this->mqttManager.publishBoilerMode(config.boiler.mode.c_str());
    this->mqttManager.publishBoilerTemperature(config.boiler.temperature);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::addFileRoutes(File dir)
{
    while (File file = dir.openNextFile())
    {
        if (file.isDirectory())
        {
            addFileRoutes(file);
        }
        else
        {
            String filePath = String(file.path());
            String contentType = getContentType(filePath);
            server.on(filePath.c_str(), HTTP_GET, [this, filePath, contentType](AsyncWebServerRequest *request)
                      { request->send(LittleFS, filePath, contentType); });
            Serial.print("  -> Route créée pour : ");
            Serial.print(filePath);
            Serial.print(" | Type : ");
            Serial.println(contentType);
        }
    }
}

void WebServerManager::setupLocalWeb()
{
    Serial.println("[-] Configuration des routes dynamiques depuis LittleFS ...");
    File root = LittleFS.open("/");
    addFileRoutes(root);

    // HTML routes
    // Route principale qui sert le fichier index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(LittleFS, "/index.html", "text/html"); });

    // L'Erreur 404 est géré par l'application Réact, on renvoi toujour index.html
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(LittleFS, "/index.html", "text/html"); });

    Serial.println("[-] Serveur Web Ok");
}

void WebServerManager::setupApiRoutes()
{
    // API routes
    server.on("/saveWifiSettings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { handleSaveWifiSettings(request, data, len); });

    server.on("/saveMqttSettings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { handleSaveMqttSettings(request, data, len); });
    server.on("/saveSolarSettings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { handleSaveSolarSettings(request, data, len); });
    server.on("/saveBoilerSettings", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
              { handleSaveBoilerSettings(request, data, len); });

    server.on("/getConfig", HTTP_GET, [this](AsyncWebServerRequest *request)
              { handleGetConfig(request); });

    server.on("/reboot", HTTP_POST, [this](AsyncWebServerRequest *request)
              { handleReboot(request); });

    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
               { onWsEvent(server, client, type, arg, data, len); });
    server.addHandler(&ws);
}

void WebServerManager::startServer()
{
    setupLocalWeb();
    setupApiRoutes();
    server.begin();
    Serial.println("[-] Serveur Web Ok");
}

void WebServerManager::broadcastData(float temperature, float triacOpeningPercentage, bool temperatureReached)
{
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["triacOpeningPercentage"] = triacOpeningPercentage;
    doc["temperatureReached"] = temperatureReached;

    String currentJson;
    serializeJson(doc, currentJson);

    // Send data if it has changed or if a new client connected
    if (newClientConnected || currentJson != lastBroadcastedJson)
    {
        ws.textAll(currentJson);
        lastBroadcastedJson = currentJson;
        newClientConnected = false;
    }
}

// Fonction pour déterminer le Content - Type(MIME type) d'un fichier en fonction de son extension
String WebServerManager::getContentType(String filename)
{
    if (filename.endsWith(".html") || filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "text/javascript";
    else if (filename.endsWith(".json"))
        return "application/json";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    return "text/plain";
}