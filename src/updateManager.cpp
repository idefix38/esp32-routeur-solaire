#include "updateManager.h"
#include "version.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "mbedtls/sha256.h"

// --- Constantes ---
const char *GITHUB_REPO = "idefix38/esp32-routeur-solaire";
const char *GITHUB_API_URL = "https://api.github.com/repos/idefix38/esp32-routeur-solaire/releases/latest";

const char *github_root_ca = R"EOF(
-----BEGIN CERTIFICATE-----
MIICjzCCAhWgAwIBAgIQXIuZxVqUxdJxVt7NiYDMJjAKBggqhkjOPQQDAzCBiDEL
MAkGA1UEBhMCVVMxEzARBgNVBAgTCk5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNl
eSBDaXR5MR4wHAYDVQQKExVUaGUgVVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMT
JVVTRVJUcnVzdCBFQ0MgQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkwHhcNMTAwMjAx
MDAwMDAwWhcNMzgwMTE4MjM1OTU5WjCBiDELMAkGA1UEBhMCVVMxEzARBgNVBAgT
Ck5ldyBKZXJzZXkxFDASBgNVBAcTC0plcnNleSBDaXR5MR4wHAYDVQQKExVUaGUg
VVNFUlRSVVNUIE5ldHdvcmsxLjAsBgNVBAMTJVVTRVJUcnVzdCBFQ0MgQ2VydGlm
aWNhdGlvbiBBdXRob3JpdHkwdjAQBgcqhkjOPQIBBgUrgQQAIgNiAAQarFRaqflo
I+d61SRvU8Za2EurxtW20eZzca7dnNYMYf3boIkDuAUU7FfO7l0/4iGzzvfUinng
o4N+LZfQYcTxmdwlkWOrfzCjtHDix6EznPO/LlxTsV+zfTJ/ijTjeXmjQjBAMB0G
A1UdDgQWBBQ64QmG1M8ZwpZ2dEl23OA1xmNjmjAOBgNVHQ8BAf8EBAMCAQYwDwYD
VR0TAQH/BAUwAwEB/zAKBggqhkjOPQQDAwNoADBlAjA2Z6EWCNzklwBBHU6+4WMB
zzuqQhFkoJ2UOQIReVx7Hfpkue4WQrO/isIJxOzksU0CMQDpKmFHjFJKS04YcPbW
RNZu9YO6bVi9JNlWSOrvxKJGgYhqOkbRqZtNyWHa0V1Xahg=
-----END CERTIFICATE-----
)EOF";

UpdateManager::UpdateManager() {}

/**
 * @brief Compare deux chaînes de version sémantique (ex: "V1.2.3").
 * @return 1 si v1 > v2, -1 si v1 < v2, 0 si elles sont égales.
 */
int compareVersions(String v1, String v2)
{
    int v1_major = 0, v1_minor = 0, v1_patch = 0;
    int v2_major = 0, v2_minor = 0, v2_patch = 0;

    // Extrait les numéros de version des chaînes (ignore le 'V' initial)
    sscanf(v1.c_str(), "V%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2.c_str(), "V%d.%d.%d", &v2_major, &v2_minor, &v2_patch);

    if (v1_major > v2_major)
        return 1;
    if (v1_major < v2_major)
        return -1;
    if (v1_minor > v2_minor)
        return 1;
    if (v1_minor < v2_minor)
        return -1;
    if (v1_patch > v2_patch)
        return 1;
    if (v1_patch < v2_patch)
        return -1;
    return 0;
}

/**
 * @brief Interroge l'API GitHub pour vérifier si une nouvelle version est disponible.
 * @return Une chaîne JSON contenant les informations de la nouvelle version, ou un JSON vide "{}" si aucune mise à jour n'est disponible ou en cas d'erreur.
 */
String UpdateManager::checkForUpdates()
{
    // Crée un client WiFi sécurisé pour la requête HTTPS
    WiFiClientSecure client;
    client.setCACert(github_root_ca);

    HTTPClient http;
    Serial.println("[Update] Checking for new version on GitHub...");
    http.begin(client, GITHUB_API_URL);

    int httpCode = http.GET();

    // Vérifie si la requête a réussi
    if (httpCode != HTTP_CODE_OK)
    {
        http.end();
        Serial.printf("[Update] HTTP GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        return "{}";
    }

    // Filter to extract only necessary fields from the JSON response
    JsonDocument filter;
    filter["tag_name"] = true;
    filter["assets"][0]["name"] = true;
    filter["assets"][0]["browser_download_url"] = true;
    filter["assets"][0]["digest"] = true;

    // Parse the JSON response with the filter
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end(); // End HTTP connection as soon as possible

    if (error)
    {
        Serial.printf("[Update] deserializeJson() failed: %s\n", error.c_str());
        return "{}";
    }

    String latest_version = doc["tag_name"];

    // Compare la version de la release avec la version actuelle du firmware
    if (compareVersions(latest_version, FIRMWARE_VERSION) <= 0)
    {
        Serial.println("[Update] No new version available.");
        return "{}";
    }

    Serial.printf("[Update] New version available: %s\n", latest_version.c_str());

    // Construit la réponse JSON pour le frontend
    JsonDocument responseDoc;
    JsonObject response = responseDoc.to<JsonObject>();
    response["new_version"] = latest_version;

    // Cherche les URLs des assets (fichiers .bin) et le sha256 dans la release
    JsonArray assets = doc["assets"];
    const String PREFIXE = "sha256:";

    for (JsonObject asset : assets)
    {
        String name = asset["name"];
        String url = asset["browser_download_url"];
        String sha256 = asset["digest"].as<String>().substring(PREFIXE.length());

        if (name.endsWith(".bin"))
        {
            if (name.startsWith("firmware-"))
            {
                response["firmware_url"] = url;
                response["firmware_sha256"] = sha256;
            }
            else if (name.startsWith("web-filesystem-"))
            {
                response["filesystem_url"] = url;
                response["filesystem_sha256"] = sha256;
            }
        }
    }

    String jsonResponse;
    serializeJson(responseDoc, jsonResponse);
    return jsonResponse;
}

bool UpdateManager::performUpdate(const String &url, const String &sha256, int command)
{
    HTTPClient httpClient;

    Serial.printf("[Update] Expected SHA256: %s\n", sha256.c_str());
    if (sha256.length() != 64) // SHA256 is 64 hex characters
    {
        Serial.printf("[Update] Invalid SHA256 length: %d\n", sha256.length());
        return false;
    }

    // --- Téléchargement et flashage du binaire ---
    httpClient.begin(url);
    int httpCode = httpClient.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[Update] Failed to download binary file: %s\n", httpClient.errorToString(httpCode).c_str());
        httpClient.end();
        return false;
    }

    int contentLength = httpClient.getSize();
    if (!Update.begin(contentLength, command))
    {
        Update.printError(Serial);
        httpClient.end();
        return false;
    }

    // Écriture du binaire en streaming pour économiser la RAM
    WiFiClient *stream = httpClient.getStreamPtr();

    // SHA256 calculation setup
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts_ret(&sha256_ctx, 0); // 0 for SHA256

    size_t written = 0;
    uint8_t buff[1024]; // Buffer for reading stream
    while (stream->available() && written < contentLength)
    {
        size_t toRead = stream->available();
        if (toRead > contentLength - written)
        {
            toRead = contentLength - written;
        }
        if (toRead > sizeof(buff))
        {
            toRead = sizeof(buff);
        }
        stream->readBytes(buff, toRead);
        Update.write(buff, toRead);
        mbedtls_sha256_update_ret(&sha256_ctx, buff, toRead);
        written += toRead;
    }

    if (written != contentLength)
    {
        Serial.printf("[Update] Written only : %d/%d. Aborting.\n", written, contentLength);
        Update.abort();
        httpClient.end();
        mbedtls_sha256_free(&sha256_ctx);
        return false;
    }

    unsigned char calculatedSha256[32]; // 32 bytes for SHA256
    mbedtls_sha256_finish_ret(&sha256_ctx, calculatedSha256);
    mbedtls_sha256_free(&sha256_ctx);

    char calculatedSha256Hex[65]; // 64 hex chars + null terminator
    for (int i = 0; i < 32; i++)
    {
        sprintf(&calculatedSha256Hex[i * 2], "%02x", calculatedSha256[i]);
    }
    calculatedSha256Hex[64] = '\0';

    Serial.printf("[Update] Calculated SHA256: %s\n", calculatedSha256Hex);

    if (sha256.equalsIgnoreCase(calculatedSha256Hex))
    {
        Serial.println("[Update] SHA256 checksum matches.");
    }
    else
    {
        Serial.println("[Update] SHA256 checksum mismatch! Aborting.");
        Update.abort();
        httpClient.end();
        return false;
    }

    // Finalise la mise à jour
    if (!Update.end())
    {
        Update.printError(Serial);
        return false;
    }

    Serial.printf("[Update] %s update successful.\n", (command == U_FLASH) ? "Firmware" : "Filesystem");
    httpClient.end();
    return true;
}

/**
 * @brief Lance le processus de mise à jour OTA.
 */
void UpdateManager::startUpdate()
{
    String jsonResponse = checkForUpdates();
    if (jsonResponse == "{}")
    {
        Serial.println("[Update] No new version available or check failed.");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error)
    {
        Serial.printf("[Update] deserializeJson() failed: %s\n", error.c_str());
        return;
    }

    const char *firmwareUrl = doc["firmware_url"];
    const char *firmwareSha256 = doc["firmware_sha256"];
    const char *filesystemUrl = doc["filesystem_url"];
    const char *filesystemSha256 = doc["filesystem_sha256"];

    Serial.println("[Update] Starting OTA update...");

    // 1. Mise à jour du système de fichiers
    if (filesystemUrl && filesystemSha256)
    {
        Serial.println("[Update] Updating filesystem...");
        if (!performUpdate(filesystemUrl, filesystemSha256, U_SPIFFS))
        {
            Serial.println("[Update] Filesystem update failed!");
            return; // Arrêter si la mise à jour du FS échoue
        }
    }

    // 2. Mise à jour du firmware applicatif
    if (firmwareUrl && firmwareSha256)
    {
        Serial.println("[Update] Updating firmware...");
        if (!performUpdate(firmwareUrl, firmwareSha256, U_FLASH))
        {
            Serial.println("[Update] Firmware update failed!");
            return; // La mise à jour du firmware a échoué
        }
    }

    Serial.println("[Update] Update successful! Rebooting...");
    ESP.restart();
}