#include "WifiManager.h"

#define LED_BUILTIN 2

// Constructeur : initialise le SSID pour le mode AP, ainsi que le SSID et le mot de passe pour la connexion STA
WifiManager::WifiManager() {}

// Configure le mode point d'accès (AP)
void WifiManager::setupAccessPoint(const char *ssid_ap)
{
    Serial.print("[-] SetUp Access Point ");
    Serial.println(ssid_ap);
    // Allume la LED interne
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    WiFi.mode(WIFI_MODE_APSTA); // Définit explicitement le mode STA+AP
    WiFi.softAP(ssid_ap);
    Serial.print("    - Adresse IP en Mode AP : ");
    Serial.println(WiFi.softAPIP());
    WiFi.setHostname(ssid_ap);
}

// Connecte l'ESP32 à un réseau WiFi en utilisant le SSID et le mot de passe fournis
String WifiManager::connect(const char *_ssid, const char *_password, int timeout)
{
    ssid = _ssid;
    password = _password;

    Serial.println("[-] Connexion au WiFi ...");
    int i = 0;
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED && i < timeout)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        Serial.print(".");
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
        i++;
    }
    Serial.println(".");

    if (WiFi.status() == WL_CONNECTED)
    {
        // Désactive le mode AP
        WiFi.mode(WIFI_MODE_STA);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("[-] Connexion au WiFi OK!");
        Serial.print("    - Adresse IP : http://");
        Serial.println(WiFi.localIP());
        return WiFi.localIP().toString();
    }

    return ""; // Retourne une chaîne vide si la connexion échoue
}
