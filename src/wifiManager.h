#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>

class WifiManager
{
public:
    // Constructeur
    WifiManager();

    // Méthodes pour configurer le WiFi en mode AP
    void setupAccessPoint(const char *ssid_ap);

    // Méthode de connexion
    String connect(const char *ssid, const char *password, int timeout = 10);

private:
    const char *ssid_ap;  // SSID pour le mode AP
    const char *ssid;     // SSID pour le mode STA
    const char *password; // Mot de passe pour le mode STA
};

#endif
