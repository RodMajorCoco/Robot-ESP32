#include "robot_functions.h"


// fonction pour mettre à jour l'affichage OLED
void updateOLED(String line1, String line2) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.setTextSize(1);
    display.println(line1);
    display.setCursor(0, 30);
    display.setTextSize(2);
    display.println(line2);
    display.display();
}

// fonction pour sauvegarder les credentials WiFi dans les Preferences
void saveWifiCredentials(const char* ssid, const char* pass) {
    preferences.begin("wifi-gate", false); 
    preferences.putString("ssid", ssid);
    preferences.putString("password", pass);
    preferences.end();
}

// fonction pour sauvegarder les credentials Web dans les Preferences
void saveAuthCredentials(const char* user, const char* pass) {
    preferences.begin("wifi-gate", false); 
    preferences.putString("web_user", user);
    preferences.putString("web_pass", pass);
    preferences.end();
}

// Fonction factorisée pour vérifier l'authentification
bool isAuthenticated(AsyncWebServerRequest *request) {
    if (!request->authenticate(www_username.c_str(), www_password.c_str())) {
        request->requestAuthentication();
        return false;
    }
    return true;
}

// Fonction pour mettre à jour l'action courante
void setAction(const char* action) {
    current_action = action;
    lastCommandTime = millis();
    updateOLED("ROBOT S3 READY", current_action);
}