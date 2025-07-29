#include "WebServerManager.h"

// Constructeur
WebServerManager::WebServerManager(ConfigManager &configManager)
    : configManager(configManager), server(80)
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

    if (strlen(shellyEmIp) == 0 || strlen(shellyEmChannel) == 0)
    {
        Serial.println("Erreur : shellyEmIp ou shellyEmChannel vide !");
        request->send(400, "application/json", "{\"status\":\"Invalid Shelly EM settings\"}");
        return;
    }

    Config config = this->configManager.loadConfig();
    config.shellyEmIp = shellyEmIp;
    config.shellyEmChannel = shellyEmChannel;
    config.boilerMode = boilerMode;
    this->configManager.saveConfig(config);

    request->send(200, "application/json", "{\"status\":\"success\"}");
}

void WebServerManager::setupLocalWeb()
{

    // Ouvre le répertoire racine de SPIFFS
    File root = SPIFFS.open("/");
    if (!root)
    {
        Serial.println("Erreur à l'ouverture du répertoire racine de SPIFFS");
        return;
    }
    Serial.println("[-] Configuration des routes dynamiques depuis SPIFFS...");
    File file = root.openNextFile();

    // Itère sur chaque fichier du répertoire
    while (file)
    {
        // Construit le chemin complet du fichier
        String filePath = String(file.name());
        if (!filePath.startsWith("/"))
        {
            filePath = "/" + filePath;
        }

        // Détermine le type de contenu (MIME)
        String contentType = getContentType(filePath);

        // Crée une route pour servir ce fichier
        // On capture les variables filePath et contentType par valeur pour la fonction lambda
        server.on(filePath.c_str(), HTTP_GET, [filePath, contentType](AsyncWebServerRequest *request)
                  { request->send(SPIFFS, filePath, contentType); });

        Serial.print("  -> Route créée pour : ");
        Serial.print(filePath);
        Serial.print(" | Type : ");
        Serial.println(contentType);

        file.close();
        file = root.openNextFile();
    }
    root.close();
    // HTML routes
    // Route principale qui sert le fichier index.html
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

    // L'Erreur 404 est géré par l'application Réact, on renvoi toujour index.html
    server.onNotFound([](AsyncWebServerRequest *request)
                      { request->send(SPIFFS, "/index.html", "text/html"); });

    server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
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

    server.on("/getTemperature", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                      float temperature = getTemperature();
                      request->send(200, "application/json", "{\"temperature\":\"" + String(temperature) + "\"}"); });

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