#include "updateManager.h"
#include "version.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "mbedtls/sha256.h"

// --- Constantes ---
const char* GITHUB_REPO = "idefix38/esp32-routeur-solaire";
const char* GITHUB_API_URL = "https://api.github.com/repos/idefix38/esp32-routeur-solaire/releases/latest";

// Certificat racine pour api.github.com (DigiCert High Assurance EV Root CA)
const char* github_root_ca =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n"
    "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
    "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n"
    "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n"
    "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
    "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n"
    "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhUcqHRMMhtL8LC75PyT9o0yuAODhA2N2N\n"
    "aEo+5vo6p0RbL/y23vYj43+sSYkC6AZR6CU3K2cE/RJqn03Xj40I+TCp900e0wG\n"
    "EonRBC0Eh6tP/e2f3XbC8F+2v3sWc+sH/fcw+Y/6cR0sT/c9x/f+tZ4eTIGNh/V\n"
    "s1/J/k5sOZAeT1bJz3N4fls5oXroA41Ld2t3C3AAb/vAF9E9+N2j/Cj3c4d/G3/l\n"
    "e3R5v0pbkZucr/18i+q032/3A4333s+4k4A245/s+3d82/5d/u4+21jMn04a2z6\n"
    "p/8t2aT/8E5o+hR2qaC+vA==\n"
    "-----END CERTIFICATE-----\n";

UpdateManager::UpdateManager() {}

/**
 * @brief Compare deux chaînes de version sémantique (ex: "V1.2.3").
 * @return 1 si v1 > v2, -1 si v1 < v2, 0 si elles sont égales.
 */
int compareVersions(String v1, String v2) {
    int v1_major = 0, v1_minor = 0, v1_patch = 0;
    int v2_major = 0, v2_minor = 0, v2_patch = 0;

    // Extrait les numéros de version des chaînes (ignore le 'V' initial)
    sscanf(v1.c_str(), "V%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2.c_str(), "V%d.%d.%d", &v2_major, &v2_minor, &v2_patch);

    if (v1_major > v2_major) return 1;
    if (v1_major < v2_major) return -1;
    if (v1_minor > v2_minor) return 1;
    if (v1_minor < v2_minor) return -1;
    if (v1_patch > v2_patch) return 1;
    if (v1_patch < v2_patch) return -1;
    return 0;
}

/**
 * @brief Interroge l'API GitHub pour vérifier si une nouvelle version est disponible.
 * @return Une chaîne JSON contenant les informations de la nouvelle version, ou un JSON vide "{}" si aucune mise à jour n'est disponible ou en cas d'erreur.
 */
String UpdateManager::checkForUpdates() {
    // Crée un client WiFi sécurisé pour la requête HTTPS
    WiFiClientSecure client;
    client.setCACert(github_root_ca);

    HTTPClient http;
    Serial.println("[Update] Checking for new version on GitHub...");
    http.begin(client, GITHUB_API_URL);

    int httpCode = http.GET();

    // Vérifie si la requête a réussi
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        Serial.printf("[Update] HTTP GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        return "{}";
    }

    // Parse la réponse JSON (limité à 2048 octets pour économiser la mémoire)
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, http.getStream());
    if (error) {
        Serial.printf("[Update] deserializeJson() failed: %s\n", error.c_str());
        http.end();
        return "{}";
    }

    String latest_version = doc["tag_name"];
    http.end();

    // Compare la version de la release avec la version actuelle du firmware
    if (compareVersions(latest_version, FIRMWARE_VERSION) <= 0) {
        Serial.println("[Update] No new version available.");
        return "{}";
    }

    Serial.printf("[Update] New version available: %s\n", latest_version.c_str());

    // Construit la réponse JSON pour le frontend
    JsonObject response = doc.to<JsonObject>();
    response["new_version"] = latest_version;

    // Cherche les URLs des assets (fichiers .bin et .md5) dans la release
    JsonArray assets = doc["assets"];
    for (JsonObject asset : assets) {
        String name = asset["name"];
        String url = asset["browser_download_url"];

        if (name.endsWith(".bin")) {
            if (name.startsWith("firmware-")) {
                response["firmware_url"] = url;
            } else if (name.startsWith("web-filesystem-")) {
                response["spiffs_url"] = url;
            }
        } else if (name.endsWith(".sha256")) { // On cherche maintenant le .sha256
            if (name.startsWith("firmware-")) {
                response["firmware_sha256_url"] = url;
            } else if (name.startsWith("web-filesystem-")) {
                response["spiffs_sha256_url"] = url;
            }
        }
    }
    
    String jsonResponse;
    serializeJson(response, jsonResponse);
    return jsonResponse;
}

bool UpdateManager::performUpdate(const String& url, const String& sha256Url, int command) {
    HTTPClient httpClient;

    // --- Téléchargement du checksum SHA256 ---
    httpClient.begin(sha256Url);
    int httpCode = httpClient.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[Update] Failed to download SHA256 checksum file: %s\n", httpClient.errorToString(httpCode).c_str());
        httpClient.end();
        return false;
    }
    String expectedSha256 = httpClient.getString();
    httpClient.end();
    expectedSha256.trim();

    if (expectedSha256.length() != 64) { // SHA256 is 64 hex characters
        Serial.printf("[Update] Invalid SHA256 length: %d\n", expectedSha256.length());
        return false;
    }
    Serial.printf("[Update] Expected SHA256: %s\n", expectedSha256.c_str());

    // --- Téléchargement et flashage du binaire ---
    httpClient.begin(url);
    httpCode = httpClient.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("[Update] Failed to download binary file: %s\n", httpClient.errorToString(httpCode).c_str());
        httpClient.end();
        return false;
    }

    int contentLength = httpClient.getSize();
    if (!Update.begin(contentLength, command)) {
        Update.printError(Serial);
        httpClient.end();
        return false;
    }

    // Remove Update.setMD5 as we will do manual SHA256 verification
    // Update.setMD5(md5.c_str());

    // Écriture du binaire en streaming pour économiser la RAM
    WiFiClient* stream = httpClient.getStreamPtr();
    
    // SHA256 calculation setup
    mbedtls_sha256_context sha256_ctx;
    mbedtls_sha256_init(&sha256_ctx);
    mbedtls_sha256_starts_ret(&sha256_ctx, 0); // 0 for SHA256

    size_t written = 0;
    uint8_t buff[1024]; // Buffer for reading stream
    while (stream->available() && written < contentLength) {
        size_t toRead = stream->available();
        if (toRead > contentLength - written) {
            toRead = contentLength - written;
        }
        if (toRead > sizeof(buff)) {
            toRead = sizeof(buff);
        }
        stream->readBytes(buff, toRead);
        Update.write(buff, toRead);
        mbedtls_sha256_update_ret(&sha256_ctx, buff, toRead);
        written += toRead;
    }

    if (written != contentLength) {
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
    for (int i = 0; i < 32; i++) {
        sprintf(&calculatedSha256Hex[i * 2], "%02x", calculatedSha256[i]);
    }
    calculatedSha256Hex[64] = '\0';

    Serial.printf("[Update] Calculated SHA256: %s\n", calculatedSha256Hex);

    if (expectedSha256.equalsIgnoreCase(calculatedSha256Hex)) {
        Serial.println("[Update] SHA256 checksum matches.");
    } else {
        Serial.println("[Update] SHA256 checksum mismatch! Aborting.");
        Update.abort();
        httpClient.end();
        return false;
    }

    // Finalise la mise à jour
    if (!Update.end()) {
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
void UpdateManager::startUpdate(const String& firmwareUrl, const String& firmwareSha256Url, const String& spiffsUrl, const String& spiffsSha256Url) {
    Serial.println("[Update] Starting OTA update...");

    // 1. Mise à jour du système de fichiers (SPIFFS)
    if (spiffsUrl.length() > 0) {
        Serial.println("[Update] Updating filesystem...");
        if (!performUpdate(spiffsUrl, spiffsSha256Url, U_SPIFFS)) {
            Serial.println("[Update] Filesystem update failed!");
            return; // Arrêter si la mise à jour du FS échoue
        }
    }

    // 2. Mise à jour du firmware applicatif
    if (firmwareUrl.length() > 0) {
        Serial.println("[Update] Updating firmware...");
        if (!performUpdate(firmwareUrl, firmwareSha256Url, U_FLASH)) {
            Serial.println("[Update] Firmware update failed!");
            return; // La mise à jour du firmware a échoué
        }
    }

    Serial.println("[Update] Update successful! Rebooting...");
    ESP.restart();
}
