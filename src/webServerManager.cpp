#include "WebServerManager.h"
#include "mqttManager.h"

// Constructeur
WebServerManager::WebServerManager(ConfigManager &configManager, MqttManager &mqttManager)
    : configManager(configManager), mqttManager(mqttManager), server(80)
{
}

void WebServerManager::handleGetConfig(AsyncWebServerRequest *request)
{
    Serial.println(" GET: /getConfig");
    Config config = this->configManager.loadConfig();

    JsonDocument doc;
    JsonObject wifiObj = doc["wifi"].to<JsonObject>();
    wifiObj["ssid"] = config.wifiSSID;
    wifiObj["password"] = "********";

    JsonObject mqttObj = doc["mqtt"].to<JsonObject>();
    mqttObj["server"] = config.mqttServer;
    mqttObj["port"] = config.mqttPort;
    mqttObj["username"] = config.mqttUsername;
    mqttObj["password"] = "********";
    mqttObj["topic"] = config.mqttTopic;

    JsonObject shellyObj = doc["shellyEm"].to<JsonObject>();
    shellyObj["ip"] = config.shellyEmIp;
    shellyObj["channel"] = config.shellyEmChannel;

    JsonObject boilerObj = doc["boiler"].to<JsonObject>();
    boilerObj["mode"] = config.boilerMode;
    boilerObj["temperature"] = config.boilerTemperature;

    JsonObject solarObj = doc["solar"].to<JsonObject>();
    solarObj["latitude"] = config.latitude;
    solarObj["longitude"] = config.longitude;
    solarObj["timeZone"] = config.timeZone;

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

    const char *ssid = doc["ssid"] | "";
    const char *password = doc["password"] | "";

    Config config = this->configManager.loadConfig();
    config.wifiSSID = ssid;
    if (password == "" || strcmp(password, "********") == 0)
    {
        // Ne pas modifier le mot de passe
    }
    else
    {
        config.wifiPassword = password;
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

    const char *server = doc["server"] | "";
    const char *topic = doc["topic"] | "";
    const char *mqtt_username = doc["username"] | "";
    const char *mqtt_password = doc["password"] | "";

    int mqtt_port = doc["port"] | 1883;

    Config config = this->configManager.loadConfig();
    config.mqttServer = server;
    config.mqttPort = mqtt_port;
    config.mqttTopic = topic;
    config.mqttUsername = mqtt_username;
    if (mqtt_password == "" || strcmp(mqtt_password, "********") == 0)
    {
        // Ne pas modifier le mot de passe
    }
    else
    {
        config.mqttPassword = mqtt_password;
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

    const char *shellyEmIp = doc["ip"] | "";
    const char *shellyEmChannel = doc["channel"] | "";
    const char *boilerMode = doc["mode"] | "Auto";
    int temperature = doc["temperature"] | 50;
    float latitude = doc["latitude"] | 48.8566;
    float longitude = doc["longitude"] | 2.3522;
    const char *timeZone = doc["timeZone"] | "Europe/Paris";

    if (strlen(shellyEmIp) == 0 || strlen(shellyEmChannel) == 0)
    {
        Serial.println("Erreur : shellyEmIp ou shellyEmChannel vide !");
        request->send(400, "application/json", "{\"status\":\"Invalid Shelly EM settings\"}");
        return;
    }

    Config configTmp = this->configManager.loadConfig();
    configTmp.shellyEmIp = shellyEmIp;
    configTmp.shellyEmChannel = shellyEmChannel;
    configTmp.boilerMode = boilerMode;
    configTmp.boilerTemperature = temperature;
    configTmp.latitude = latitude;
    configTmp.longitude = longitude;
    configTmp.timeZone = timeZone;
    this->configManager.saveConfig(configTmp);

    // Met à jour la variable globale config pour prise en compte immédiate dans loop()
    extern Config config;
    config = configTmp;
    // Met à jour le mode du chauffe-eau dans le gestionnaire MQTT
    this->mqttManager.publishBoilerMode(boilerMode);
    this->mqttManager.publishBoilerTemperature(temperature);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::addFileRoutes(File dir) {
    while (File file = dir.openNextFile()) {
        if (file.isDirectory()) {
            addFileRoutes(file);
        } else {
            String filePath = String(file.path());
            String contentType = getContentType(filePath);
            server.on(filePath.c_str(), HTTP_GET, [this, filePath, contentType](AsyncWebServerRequest *request) {
                request->send(LittleFS, filePath, contentType);
            });
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

    server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request) 
              {
                      extern volatile float lastTemperature;
                      extern volatile float triacOpeningPercentage;
                      request->send(200, "application/json", "{\"temperature\":\"" + String(lastTemperature) + "\", \"triacOpeningPercentage\":\"" + String(triacOpeningPercentage) + "\"}"); });

    server.on("/getConfig", HTTP_GET, [this](AsyncWebServerRequest *request) 
              { handleGetConfig(request); });
}

void WebServerManager::startServer()
{
    setupLocalWeb();
    setupApiRoutes();
    server.begin();
    Serial.println("[-] Serveur Web Ok");
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
