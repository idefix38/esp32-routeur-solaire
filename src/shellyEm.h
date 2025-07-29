#ifndef SHELLYEM_H
#define SHELLYEM_H

#include <Arduino.h>

/// @brief Classe pour interagir avec le module ShellyEM.
/// Cette classe permet de récupérer la puissance sur un channel spécifique du ShellyEM via son API REST.
class ShellyEm
{
    String ip;
    String channel;

public:
    /**
     * Constructeur de la classe ShellyEm.
     * @param adresseIp Adresse IP du ShellyEM.
     * @param channel Channel du ShellyEM à interroger (ex: "0" pour le premier channel).
     */
    ShellyEm(const String &adresseIp, const String &channel);

    /**
     * Récupère la puissance sur le channel du ShellyEM via son API REST.
     * @return Puissance (Watt) sur le channel, ou 0 en cas d'erreur.
     */
    float getPower();
};

#endif
