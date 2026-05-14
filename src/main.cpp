/************************************************************
 *  Fichier  : main.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 *  Matériel : ESP32-S3 N16R8 + DRV8833 + SSD1306
 * ----------------------------------------------------------
 *  Description :
 *    Point d'entrée. Instancie les quatre sous-systèmes,
 *    gère le boot (WiFi normal ou portail AP), puis délègue
 *    à chaque objet dans loop().
 ************************************************************/

#include <Arduino.h>
#include "config.h"
#include "motor_controller.h"
#include "oled_display.h"
#include "battery_monitor.h"
#include "wifi_manager.h"
#include "robot_server.h"

// ---------------------------------------------------------------------------
// Instances globales des sous-systèmes
// ---------------------------------------------------------------------------
MotorController motors;
OledDisplay     oled;
BatteryMonitor  battery;
WifiManager     wifi;
RobotServer     robotServer(motors, oled, battery, wifi);

// ---------------------------------------------------------------------------
void setup() {

#if DEBUG_MODE
    Serial.begin(115200);
#endif

    // --- Moteurs ---
    motors.begin();

    // --- Écran OLED ---
    oled.begin();
    oled.update("ROBOT S3 READY", "BOOTING...", 100);

    // --- WiFi ---
    bool connected = wifi.connect();

    if (!connected) {
        // Credentials absents ou connexion échouée → portail AP
        robotServer.beginAP();
        return; // loop() se chargera du timeout AP
    }

    oled.update("CONNECTE", WiFi.localIP().toString(), battery.getPercent());

    // --- Credentials interface web ---
    String webUser, webPass;
    wifi.loadAuthCredentials(webUser, webPass);

    // --- Serveur télécommande ---
    robotServer.beginNormal(webUser, webPass);

    // --- Batterie ---
    battery.begin();
    battery.update();

    // --- Timer watchdog ---
    // (géré via MotorController::getLastCommandTime dans loop)
    oled.update("ROBOT S3 READY", "STOP", battery.getPercent());
}

// ---------------------------------------------------------------------------
void loop() {

    // --- Mode AP : surveiller uniquement le timeout ---
    if (robotServer.isAPMode()) {
        robotServer.handleAPTimeout();
        return;
    }

    // --- Watchdog WiFi ---
    if (!wifi.isConnected()) {
        oled.update("ERREUR WIFI", "reconnexion...", battery.getPercent());
        wifi.handleReconnect();
    }

    // --- Watchdog de sécurité (arrêt si pas de commande depuis > 5 s) ---
    Action current = motors.getCurrentAction();
    if (current != Action::STOP &&
        (millis() - motors.getLastCommandTime() > MAX_PERIOD_WITHOUT_COMMAND)) {

        motors.setAction(Action::STOP);
        oled.update("ROBOT S3 READY",
                    MotorController::actionToString(Action::STOP),
                    battery.getPercent());
    }

    // --- Mise à jour batterie (toutes les BATTERY_READ_INTERVAL ms) ---
    static unsigned long lastBatteryRead = 0;
    if (millis() - lastBatteryRead > BATTERY_READ_INTERVAL) {
        battery.update();
        lastBatteryRead = millis();
    }
}
