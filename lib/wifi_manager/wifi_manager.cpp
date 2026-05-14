/************************************************************
 *  Fichier  : wifi_manager.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 ************************************************************/

#include "wifi_manager.h"

// ---------------------------------------------------------------------------
WifiManager::WifiManager()
    : _lastReconnectAttempt(0)
{}

// ---------------------------------------------------------------------------
bool WifiManager::connect() {
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    String ssid = prefs.getString("ssid",     "");
    String pass = prefs.getString("password", "");
    prefs.end();

    if (ssid.isEmpty() || pass.isEmpty()) {
        #if DEBUG_MODE
        Serial.println("[WIFI] Credentials absents");
        #endif
        return false;
    }

    WiFi.begin(ssid.c_str(), pass.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_WIFI_RETRIES) {
        delay(500);
        attempts++;
        #if DEBUG_MODE
        Serial.printf("[WIFI] Tentative %d/%d\n", attempts, MAX_WIFI_RETRIES);
        #endif
    }

    if (WiFi.status() == WL_CONNECTED) {
        #if DEBUG_MODE
        Serial.print("[WIFI] Connecté — IP : ");
        Serial.println(WiFi.localIP());
        #endif
        return true;
    }

    #if DEBUG_MODE
    Serial.println("[WIFI] Connexion échouée");
    #endif
    return false;
}

// ---------------------------------------------------------------------------
void WifiManager::handleReconnect() {
    if (isConnected()) return;

    unsigned long now = millis();
    if (now - _lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) return;

    _lastReconnectAttempt = now;

    #if DEBUG_MODE
    Serial.println("[WIFI] Perdu — tentative de reconnexion");
    #endif

    WiFi.reconnect();
}

// ---------------------------------------------------------------------------
void WifiManager::saveCredentials(const char* ssid, const char* pass) {
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString("ssid",     ssid);
    prefs.putString("password", pass);
    prefs.end();
}

// ---------------------------------------------------------------------------
void WifiManager::saveAuthCredentials(const char* user, const char* pass) {
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, false);
    prefs.putString("web_user", user);
    prefs.putString("web_pass", pass);
    prefs.end();
}

// ---------------------------------------------------------------------------
void WifiManager::loadAuthCredentials(String& outUser, String& outPass) {
    Preferences prefs;
    prefs.begin(PREFS_NAMESPACE, true);
    outUser = prefs.getString("web_user", "ERR");
    outPass = prefs.getString("web_pass", "ERR");
    prefs.end();
}
