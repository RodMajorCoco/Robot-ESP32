/************************************************************
 *  Fichier  : wifi_manager.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe WifiManager — gestion de la connexion WiFi en
 *    mode STA, lecture / écriture des credentials dans les
 *    Preferences, watchdog de reconnexion dans loop().
 *    Ne gère PAS le portail AP (délégué à RobotServer).
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
     * Tente de se connecter avec les credentials stockés.
     * @return true si connecté, false si credentials absents ou timeout.
     */
    bool connect();

    /**
     * À appeler dans loop() : tente une reconnexion si la connexion
     * est perdue, en respectant WIFI_RECONNECT_INTERVAL.
     */
    void handleReconnect();

    /** Sauvegarde les credentials WiFi dans les Preferences. */
    void saveCredentials(const char* ssid, const char* pass);

    /** Sauvegarde les credentials de l'interface web dans les Preferences. */
    void saveAuthCredentials(const char* user, const char* pass);

    /** Charge le login/mdp web depuis les Preferences. */
    void loadAuthCredentials(String& outUser, String& outPass);

    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }

private:
    unsigned long _lastReconnectAttempt;
};

#endif // WIFI_MANAGER_H
