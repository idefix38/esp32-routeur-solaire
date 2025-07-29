#include "shellyEm.h"
#include <HTTPClient.h>

ShellyEm::ShellyEm(const String &adresseIp, const String &channel)
    : ip(adresseIp), channel(channel) {}

/**
 * Récupère la puissance sur le channel du ShellyEM via son API REST.
 * @return Puissance (Watt) sur le channel, ou 0 en cas d'erreur.
 */
float ShellyEm::getPower()
{
    if (ip == "" || channel == "")
        return 0;
    String url = "http://" + ip + "/emeter/" + (channel == "1" ? "0" : "1");
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode == 200)
    {
        String payload = http.getString();
        int idx = payload.indexOf("\"power\":");
        if (idx != -1)
        {
            int start = idx + 8;
            int end = payload.indexOf(',', start);
            String powerStr = payload.substring(start, end);
            http.end();
            return powerStr.toFloat();
        }
    }
    http.end();
    return 0;
}
