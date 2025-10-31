#include "wifiManager.h"
#include <ESPmDNS.h>
#include <WiFiUdp.h>

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

/**
 * Scanne le réseau local pour trouver l'adresse IP d'un device dont le nom commence par un préfixe donné,
 * en interrogeant un service mDNS spécifique.
 * @param prefix Le préfixe du nom du device à rechercher (ex: "shellyEm-")
 * @param service Le service mDNS à interroger (ex: "_http")
 * @param proto Le protocole du service mDNS (ex: "_tcp")
 * @return L'adresse IP du device trouvé, ou une chaîne vide si non trouvé.
 */
String WifiManager::findDeviceIpByNamePrefix(const String &prefix, const char *service, const char *proto)
{
    // Il est important de démarrer mDNS avant de l'utiliser pour les requêtes.
    // Le nom "esp32-scanner" est juste un identifiant pour votre ESP32 sur le réseau mDNS.
    if (!MDNS.begin("esp32-scanner"))
    {
        Serial.println("Erreur: Impossible de démarrer mDNS.");
        return "";
    }
    Serial.println("mDNS démarré.");

    // Effectuer la requête pour le service spécifié.
    // Pour Shelly, le service HTTP est généralement "_http" avec le protocole "_tcp".
    // La requête est bloquante pour un court instant, le temps de recevoir les réponses.
    int n = MDNS.queryService(service, proto);

    if (n == 0)
    {
        Serial.printf("Aucun service %s.%s trouvé.\n", service, proto);
    }
    else
    {
        Serial.printf("%d service(s) %s.%s trouvé(s):\n", n, service, proto);
        for (int i = 0; i < n; ++i)
        {
            // C'est ici que la correction s'applique :
            // Utilisez directement MDNS.getHostname(), MDNS.IP(), MDNS.getPort()
            String hostname = MDNS.hostname(i);
            IPAddress ip = MDNS.IP(i);
            int port = MDNS.port(i);

            Serial.printf("  [%d] Hôte: %s (%s:%d)\n", i + 1, hostname.c_str(), ip.toString().c_str(), port);

            // Vérifier si le nom d'hôte commence par le préfixe désiré
            if (hostname.startsWith(prefix))
            {
                Serial.printf("  -> Appareil '%s' trouvé à l'IP: %s\n", hostname.c_str(), ip.toString().c_str());
                MDNS.end(); // Arrêter mDNS après avoir trouvé l'appareil (optionnel, mais bonne pratique)
                return ip.toString();
            }
        }
    }

    MDNS.end(); // Arrêter mDNS si rien n'est trouvé ou après le traitement
    return "";  // Retourne une chaîne vide si l'appareil n'est pas trouvé
}
