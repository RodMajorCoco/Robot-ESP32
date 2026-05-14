/************************************************************
 *  Fichier  : wifi_manager.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe WifiManager — connexion WiFi en mode station (STA),
 *    persistance des credentials dans les Preferences NVS,
 *    et watchdog de reconnexion automatique dans loop().
 *    La création du portail AP est déléguée à RobotServer.
 ************************************************************/

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include "config.h"

// ---------------------------------------------------------------------------
class WifiManager {
public:
    WifiManager();

    /**
     * Charge les credentials depuis les Preferences et tente la
     * connexion WiFi (MAX_WIFI_RETRIES × 500 ms max).
     * @return  true si connecté, false si credentials absents ou timeout.
     */
    bool connect();

    /**
     * Tente une reconnexion si la connexion est perdue, en respectant
     * l'intervalle WIFI_RECONNECT_INTERVAL entre deux tentatives.
     * À appeler à chaque itération de loop().
     */
    void handleReconnect();

    /**
     * Écrit le SSID et le mot de passe WiFi dans les Preferences NVS.
     * @param ssid  Nom du réseau WiFi.
     * @param pass  Mot de passe du réseau WiFi.
     */
    void saveCredentials(const char* ssid, const char* pass);

    /**
     * Écrit le login et le mot de passe de l'interface web dans
     * les Preferences NVS.
     * @param user  Nom d'utilisateur HTTP-Basic.
     * @param pass  Mot de passe HTTP-Basic.
     */
    void saveAuthCredentials(const char* user, const char* pass);

    /**
     * Charge le login et le mot de passe de l'interface web depuis
     * les Preferences NVS.
     * @param outUser  Référence remplie avec le nom d'utilisateur.
     * @param outPass  Référence remplie avec le mot de passe.
     */
    void loadAuthCredentials(String& outUser, String& outPass);

    /**
     * Retourne true si le WiFi est actuellement connecté.
     */
    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

private:
    unsigned long _lastReconnectAttempt;
};

#endif // WIFI_MANAGER_H
