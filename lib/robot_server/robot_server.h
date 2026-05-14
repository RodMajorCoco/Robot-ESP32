/************************************************************
 *  Fichier  : robot_server.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe RobotServer — configure et démarre le serveur
 *    web asynchrone (ESPAsyncWebServer).
 *
 *    Deux modes :
 *      • Mode normal  : télécommande + endpoints /battery,
 *                       /display/on-off, /driver/on-off.
 *      • Mode AP      : portail de configuration WiFi avec
 *                       scan, sauvegarde et reboot.
 *
 *    Dépend de MotorController, OledDisplay, BatteryMonitor
 *    et WifiManager ; reçoit leurs références à l'init.
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
    RobotServer(MotorController& motors,
                OledDisplay&     display,
                BatteryMonitor&  battery,
                WifiManager&     wifi);

    /**
     * Configure les routes de la télécommande et démarre le serveur.
     * @param username  Login HTTP-Basic
     * @param password  Mot de passe HTTP-Basic
     */
    void beginNormal(const String& username, const String& password);

    /**
     * Configure les routes du portail AP et démarre le serveur.
     * Met le WiFi en mode WIFI_AP_STA et crée le soft-AP.
     */
    void beginAP();

    /** À appeler dans loop() — surveille le timeout du mode AP. */
    void handleAPTimeout();

    bool isAPMode()  const { return _apMode; }

private:
    AsyncWebServer  _server;
    MotorController& _motors;
    OledDisplay&     _display;
    BatteryMonitor&  _battery;
    WifiManager&     _wifi;

    String _username;
    String _password;
    volatile bool _apMode;
    unsigned long _apStartTime;

    bool _authenticate(AsyncWebServerRequest* request);
};

#endif // ROBOT_SERVER_H
