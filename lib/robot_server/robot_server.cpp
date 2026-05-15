/************************************************************
 *  Fichier  : robot_server.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 ************************************************************/

#include "robot_server.h"
#include "web_interface.h"
#include "web_ap.h"

// ---------------------------------------------------------------------------
RobotServer::RobotServer(MotorController& motors,
                         OledDisplay&     display,
                         BatteryMonitor&  battery,
                         WifiManager&     wifi)
    : _server(80)
    , _motors(motors)
    , _display(display)
    , _battery(battery)
    , _wifi(wifi)
    , _apMode(false)
    , _apStartTime(0)
    , _pendingRestart(false)
    , _pendingRestartTime(0)
{}

// ---------------------------------------------------------------------------
// Authentification HTTP-Basic
// ---------------------------------------------------------------------------
bool RobotServer::_authenticate(AsyncWebServerRequest* request) {
    if (!request->authenticate(_username.c_str(), _password.c_str())) {
        request->requestAuthentication();
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Mode normal — télécommande
// ---------------------------------------------------------------------------
void RobotServer::beginNormal(const String& username, const String& password) {
    _username = username;
    _password = password;

    // Page principale
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
        request->send_P(200, "text/html", index_html);
    });

    // Mouvements
    auto makeMove = [this](Action action, const char* label) {
        return [this, action, label](AsyncWebServerRequest* request) {
            if (!_authenticate(request)) return;
#if DEBUG_MODE
            Serial.printf("[CMD] %s\n", label);
#endif
            _motors.setAction(action);
            request->send(200);
        };
    };

    _server.on("/forward",  HTTP_GET, makeMove(Action::AVANCER,    "forward"));
    _server.on("/backward", HTTP_GET, makeMove(Action::RECULER,    "backward"));
    _server.on("/left",     HTTP_GET, makeMove(Action::ROTATION_G, "left"));
    _server.on("/right",    HTTP_GET, makeMove(Action::ROTATION_D, "right"));
    _server.on("/stop",     HTTP_GET, makeMove(Action::STOP,       "stop"));

    // Écran OLED
    _server.on("/display/on", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
#if DEBUG_MODE
        Serial.println("[CMD] display on");
#endif
        _display.toggle(true);
        request->send(200);
    });

    _server.on("/display/off", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
#if DEBUG_MODE
        Serial.println("[CMD] display off");
#endif
        _display.toggle(false);
        request->send(200);
    });

    // Driver moteur
    _server.on("/driver/on", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
#if DEBUG_MODE
        Serial.println("[CMD] driver on");
#endif
        _motors.toggleDriver(true);
        request->send(200);
    });

    _server.on("/driver/off", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
#if DEBUG_MODE
        Serial.println("[CMD] driver off");
#endif
        _motors.toggleDriver(false);
        request->send(200);
    });

    // Vitesses moteur
    _server.on("/speed/cruise", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
        if (request->hasParam("value")) {
            _motors.setVitesseCroisiere(request->getParam("value")->value().toInt());
        }
        request->send(200);
    });

    _server.on("/speed/rotation", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
        if (request->hasParam("value")) {
            _motors.setVitesseRotation(request->getParam("value")->value().toInt());
        }
        request->send(200);
    });

    // Batterie
    _server.on("/battery", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!_authenticate(request)) return;
        request->send(200, "text/plain", String(_battery.getPercent()));
    });

    _server.begin();
}

// ---------------------------------------------------------------------------
// Mode AP — portail de configuration
// ---------------------------------------------------------------------------
void RobotServer::beginAP() {
    _apMode     = true;
    _apStartTime = millis();

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID);

    _display.update(WiFi.softAPIP().toString(), "AP MODE", 0);

#if DEBUG_MODE
    Serial.println("[AP] Portail de configuration démarré");
    Serial.print("[AP] IP : ");
    Serial.println(WiFi.softAPIP());
#endif

    // Page principale
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", index_html_AP);
    });

    // Scan WiFi
    _server.on("/scan", HTTP_GET, [](AsyncWebServerRequest* request) {
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\""  + WiFi.SSID(i) + "\","
                  + "\"rssi\":"     + WiFi.RSSI(i)  + ","
                  + "\"secure\":"   + (WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
        }
        json += "]";
        WiFi.scanDelete();
        request->send(200, "application/json", json);
    });

    // Sauvegarde des credentials + reboot
    _server.on("/save", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!request->hasParam("ssid",    true) ||
            !request->hasParam("pass",    true) ||
            !request->hasParam("webuser", true) ||
            !request->hasParam("webpass", true)) {
            request->send(400);
            return;
        }

        String ssid    = request->getParam("ssid",    true)->value();
        String pass    = request->getParam("pass",    true)->value();
        String webuser = request->getParam("webuser", true)->value();
        String webpass = request->getParam("webpass", true)->value();

        _wifi.saveCredentials(ssid.c_str(), pass.c_str());
        _wifi.saveAuthCredentials(webuser.c_str(), webpass.c_str());

#if DEBUG_MODE
        Serial.println("[AP] Credentials sauvegardés — reboot dans 1s");
#endif

        request->send(200);
        // Le reboot est différé dans handleAPTimeout() pour laisser
        // la tâche réseau transmettre la réponse HTTP avant de redémarrer.
        _pendingRestart     = true;
        _pendingRestartTime = millis();
    });

    _server.begin();
}

// ---------------------------------------------------------------------------
// Timeout du mode AP → reboot
// ---------------------------------------------------------------------------
void RobotServer::handleAPTimeout() {
    if (!_apMode) return;

    // Reboot différé après sauvegarde des credentials
    if (_pendingRestart && millis() - _pendingRestartTime > 800) {
#if DEBUG_MODE
        Serial.println("[AP] Reboot après sauvegarde");
#endif
        _display.update("SAUVEGARDE", "Redemarrage...", 0);
        delay(500);
        ESP.restart();
    }

    // Timeout du mode AP
    if (millis() - _apStartTime > AP_CONFIG_TIMEOUT) {
#if DEBUG_MODE
        Serial.println("[AP] Timeout — reboot");
#endif
        _display.update("AP TIMEOUT", "Reboot...", 0);
        delay(500);
        ESP.restart();
    }
}
