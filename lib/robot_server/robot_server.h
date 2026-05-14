/************************************************************
 *  Fichier  : robot_server.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe RobotServer — configure et démarre le serveur
 *    web asynchrone (ESPAsyncWebServer).
 *
 *    Deux modes de fonctionnement :
 *      • Mode normal  : télécommande + endpoints /battery,
 *                       /display/on-off, /driver/on-off.
 *      • Mode AP      : portail de configuration WiFi avec
 *                       scan, sauvegarde et reboot différé.
 *
 *    Règle de sécurité : les callbacks web (core 0) ne font
 *    que mettre à jour des variables d'état. Toute écriture
 *    matérielle (I2C, PWM) est déclenchée depuis loop()
 *    sur le core 1.
 ************************************************************/

#ifndef ROBOT_SERVER_H
#define ROBOT_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include "config.h"
#include "motor_controller.h"
#include "oled_display.h"
#include "battery_monitor.h"
#include "wifi_manager.h"

// ---------------------------------------------------------------------------
class RobotServer {
public:
    /**
     * Constructeur — stocke les références aux sous-systèmes.
     * Aucun accès matériel n'est effectué ici.
     */
    RobotServer(MotorController& motors,
                OledDisplay&     display,
                BatteryMonitor&  battery,
                WifiManager&     wifi);

    /**
     * Configure les routes de la télécommande et démarre le serveur.
     * Routes : /, /forward, /backward, /left, /right, /stop,
     *          /display/on, /display/off, /driver/on, /driver/off,
     *          /battery.
     * @param username  Login HTTP-Basic pour toutes les routes.
     * @param password  Mot de passe HTTP-Basic.
     */
    void beginNormal(const String& username, const String& password);

    /**
     * Configure les routes du portail AP et démarre le serveur.
     * Met le WiFi en mode WIFI_AP_STA, crée le soft-AP (AP_SSID)
     * et affiche l'IP sur l'écran OLED.
     * Routes : /, /scan, /save (POST).
     */
    void beginAP();

    /**
     * Surveille le timeout du mode AP et le reboot différé.
     * À appeler à chaque itération de loop() quand isAPMode() = true.
     * Déclenche ESP.restart() si AP_CONFIG_TIMEOUT est atteint ou
     * si un reboot a été demandé par /save.
     */
    void handleAPTimeout();

    /**
     * Retourne true si le serveur fonctionne en mode portail AP.
     */
    bool isAPMode() const { return _apMode; }

private:
    AsyncWebServer   _server;
    MotorController& _motors;
    OledDisplay&     _display;
    BatteryMonitor&  _battery;
    WifiManager&     _wifi;

    String        _username;
    String        _password;
    volatile bool _apMode;
    unsigned long _apStartTime;
    volatile bool _pendingRestart;
    unsigned long _pendingRestartTime;

    /**
     * Vérifie les credentials HTTP-Basic de la requête.
     * Envoie automatiquement une demande d'authentification si invalide.
     * @return  true si authentifié, false sinon.
     */
    bool _authenticate(AsyncWebServerRequest* request);
};

#endif // ROBOT_SERVER_H
