#include "robot_functions.h"

SemaphoreHandle_t actionMutex;


// fonction pour mettre à jour l'affichage OLED
void updateOLED(const String& line1, const String& line2) {
    if (!isDisplayOn) return;
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
    preferences.begin(PREFS_NAMESPACE, false); 
    preferences.putString("ssid", ssid);
    preferences.putString("password", pass);
    preferences.end();
}

// fonction pour sauvegarder les credentials Web dans les Preferences
void saveAuthCredentials(const char* user, const char* pass) {
    preferences.begin(PREFS_NAMESPACE, false); 
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
void setAction(Action action) {
    if (xSemaphoreTake(actionMutex, portMAX_DELAY)) {
        if (current_action != action) {
            current_action = action;
            lastCommandTime = millis();
        }
        xSemaphoreGive(actionMutex);
    }
            
    updateOLED("ROBOT S3 READY", actionToString(current_action));
    applyMotorLogic(current_action);
}

// Fonction pour appliquer la logique des moteurs en fonction de l'action courante
void applyMotorLogic(Action action) {
    if (action == Action::AVANCER) {
        analogWrite(MOTEUR_A_IN1, VITESSE_CROISIERE);
        analogWrite(MOTEUR_A_IN2, 0);
        analogWrite(MOTEUR_B_IN1, VITESSE_CROISIERE);
        analogWrite(MOTEUR_B_IN2, 0);
    } else if (action == Action::RECULER) {
        analogWrite(MOTEUR_A_IN1, 0);
        analogWrite(MOTEUR_A_IN2, VITESSE_CROISIERE);
        analogWrite(MOTEUR_B_IN1, 0);
        analogWrite(MOTEUR_B_IN2, VITESSE_CROISIERE);
    } else if (action == Action::ROTATION_G) {
        analogWrite(MOTEUR_A_IN1, 0);
        analogWrite(MOTEUR_A_IN2, VITESSE_ROTATION);
        analogWrite(MOTEUR_B_IN1, VITESSE_ROTATION);
        analogWrite(MOTEUR_B_IN2, 0);
    } else if (action == Action::ROTATION_D) {
        analogWrite(MOTEUR_A_IN1, VITESSE_ROTATION);
        analogWrite(MOTEUR_A_IN2, 0);
        analogWrite(MOTEUR_B_IN1, 0);
        analogWrite(MOTEUR_B_IN2, VITESSE_ROTATION);
    } else if (action == Action::STOP) {
        analogWrite(MOTEUR_A_IN1, 0);
        analogWrite(MOTEUR_A_IN2, 0);
        analogWrite(MOTEUR_B_IN1, 0);
        analogWrite(MOTEUR_B_IN2, 0); 
    }    
}  

// Fonction pour convertir une action en chaîne de caractères pour l'affichage
const char* actionToString(Action a) {
    switch(a) {
        case Action::AVANCER:     return "AVANCER";
        case Action::RECULER:     return "RECULER";
        case Action::ROTATION_G:  return "ROTATION G";
        case Action::ROTATION_D:  return "ROTATION D";
        default:                  return "STOP";
    }
}

// Fonction pour basculer l'état de l'affichage OLED
void toggleDisplay(bool state) {
    isDisplayOn = state; // On met à jour l'état mémorisé
    display.ssd1306_command(state ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
    
    // Si on rallume, on force une mise à jour pour ne pas avoir un écran noir
    if (state) {
        updateOLED("ROBOT S3 READY", actionToString(current_action));
    }
}