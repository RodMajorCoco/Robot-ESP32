/************************************************************
 *  Fichier  : robot_functions.cpp
 *  Projet   : Robot ESP32-S3
 *  Auteur   : 
 *  Date     : 2026-05-01
 *  Version  : 1.1
 *  Matériel : ESP32-S3 + DRV8833 + SSD1306
 * ----------------------------------------------------------
 *  Description :
 *    Implémentation de la logique de contrôle du robot :
 *    commande PWM des moteurs via DRV8833, affichage OLED,
 *    gestion des actions avec protection mutex FreeRTOS,
 *    activation/désactivation du driver et de l'écran.
 * ----------------------------------------------------------
 *  Historique :
 *    1.0 - 2026-05-01 : Création
 *    1.1 - 2026-06-01 : Ajout de la lecture de la batterie
 ************************************************************/


#include "robot_functions.h"


SemaphoreHandle_t actionMutex; // Mutex pour protéger l'accès à current_action

// Met à jour l'affichage OLED avec deux lignes de texte
void updateOLED(const String& line1, const String& line2) {
    if (!isDisplayOn) return;
    display.clearDisplay();

    // --- Moitié haute : barre de batterie ---
    display.drawRect(0, 0, 110, 14, WHITE);       // contour
    display.fillRect(110, 4, 5, 6, WHITE);        // borne +
    int fillWidth = (int)(106.0f * batteryPercent / 100.0f);
    display.fillRect(2, 2, fillWidth, 10, WHITE);  // remplissage
    display.setTextSize(1);
    display.setTextColor(batteryPercent > 20 ? BLACK : WHITE);
    display.setCursor(38, 3);
    display.printf("%d%%", batteryPercent);

    // --- Moitié basse : action courante ---
    display.setTextColor(WHITE);
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println(line1);
    display.setCursor(0, 32);
    display.setTextSize(2);
    display.println(line2);

    display.display();
    yield(); // Laisser le temps à l'ESP de gérer d'autres tâches
}

// Sauvegarde les credentials WiFi dans les Preferences
void saveWifiCredentials(const char* ssid, const char* pass) {
    preferences.begin(PREFS_NAMESPACE, false); 
    preferences.putString("ssid", ssid);
    preferences.putString("password", pass);
    preferences.end();
}

// Sauvegarde les credentials Web dans les Preferences
void saveAuthCredentials(const char* user, const char* pass) {
    preferences.begin(PREFS_NAMESPACE, false); 
    preferences.putString("web_user", user);
    preferences.putString("web_pass", pass);
    preferences.end();
}

// Vérifie l'authentification de la requête HTTP
bool isAuthenticated(AsyncWebServerRequest *request) {
    if (!request->authenticate(www_username.c_str(), www_password.c_str())) {
        request->requestAuthentication();
        return false;
    }
    return true;
}

// Met à jour l'action courante du robot et applique la logique moteur correspondante
void setAction(Action action) {
    Action actionToApply = Action::STOP;

    if (xSemaphoreTake(actionMutex, portMAX_DELAY)) {// On protège l'accès à current_action
        current_action = action;          // toujours mettre à jour
        lastCommandTime = millis();       // toujours rafraîchir le timer
        actionToApply = current_action; 
        xSemaphoreGive(actionMutex);
    }
            
    updateOLED("ROBOT S3 READY", actionToString(actionToApply));
    applyMotorLogic(actionToApply);
}

// Applique la logique moteur en fonction de l'action demandée
void applyMotorLogic(Action action) {
    
    #if DEBUG_MODE
        Serial.print("[MOTOR] ");
        Serial.println(actionToString(action));
    #endif

    if (!isDriverOn && action != Action::STOP) {
        return; // On n'essaie même pas de piloter si le driver est OFF
    }

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
        #if BRAKE_MODE
            // Freinage actif : les deux pins à HIGH pour chaque moteur
            analogWrite(MOTEUR_A_IN1, 255);
            analogWrite(MOTEUR_A_IN2, 255);
            analogWrite(MOTEUR_B_IN1, 255);
            analogWrite(MOTEUR_B_IN2, 255); 
        #else
            // Roues libres : les deux pins à LOW pour chaque moteur    
            analogWrite(MOTEUR_A_IN1, 0);
            analogWrite(MOTEUR_A_IN2, 0);
            analogWrite(MOTEUR_B_IN1, 0);
            analogWrite(MOTEUR_B_IN2, 0); 
        #endif
    }    
}  

// Convertit une action en chaîne de caractères pour l'affichage
const char* actionToString(Action a) {
    switch(a) {
        case Action::AVANCER:     return "AVANCER";
        case Action::RECULER:     return "RECULER";
        case Action::ROTATION_G:  return "ROTATION G";
        case Action::ROTATION_D:  return "ROTATION D";
        default:                  return "STOP";
    }
}

// Active ou désactive l'affichage OLED
void toggleDisplay(bool state) {
    isDisplayOn = state; // On met à jour l'état mémorisé
    display.ssd1306_command(state ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
  
    // Si on rallume, on force une mise à jour pour ne pas avoir un écran noir
    if (state) {
        Action snapshot = Action::STOP;
        if (xSemaphoreTake(actionMutex, portMAX_DELAY)) { // On prend un snapshot de l'action courante pour l'afficher
            snapshot = current_action;
            xSemaphoreGive(actionMutex);
        }
        updateOLED("ROBOT S3 READY", actionToString(snapshot));
    }
}

// Active ou désactive le driver moteur
void toggleDriver(bool state) {
    isDriverOn = state;
    digitalWrite(DRV8833_EEP, state ? HIGH : LOW);

    if (!state) {
        // Pas de mutex ici, on appelle directement applyMotorLogic + reset action
        if (xSemaphoreTake(actionMutex, pdMS_TO_TICKS(10))) {
            current_action = Action::STOP;
            lastCommandTime = millis();
            xSemaphoreGive(actionMutex);
        }
        applyMotorLogic(Action::STOP);
        updateOLED("ROBOT S3 READY", "DRIVER OFF");
    } else {
        updateOLED("ROBOT S3 READY", "DRIVER ON");
    }
}


volatile int batteryPercent = 100;

// Lit la tension batterie et met à jour le pourcentage
void updateBattery() {
    // Moyenne sur 16 lectures pour réduire le bruit ADC
    int raw = 0;
    for (int i = 0; i < 16; i++) {
        raw += analogRead(BATTERY_ADC_PIN);
    }
    raw /= 16;

    // Conversion en tension réelle
    float vADC = (raw / BATTERY_ADC_RES) * BATTERY_ADC_REF;
    float vBat = vADC * (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;

    // Calcul du pourcentage avec clamping
    int pct = (int)(((vBat - BATTERY_VMIN) / (BATTERY_VMAX - BATTERY_VMIN)) * 100.0f);
    if (pct > 100) pct = 100;
    if (pct < 0)   pct = 0;

    batteryPercent = pct;

    #if DEBUG_MODE
        Serial.printf("[BAT] raw=%d vADC=%.3f vBat=%.3f pct=%d%%\n",
                      raw, vADC, vBat, pct);
    #endif
}


volatile bool isAPMode = false;

void startConfigPortal(AsyncWebServer& server) {
    isAPMode = true;

    // Démarrage du point d'accès
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(AP_SSID);

    updateOLED("CONFIG MODE", AP_SSID);

    #if DEBUG_MODE
        Serial.println("[AP] Portail de configuration démarré");
        Serial.print("[AP] IP : ");
        Serial.println(WiFi.softAPIP());
    #endif

    // Page de configuration
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", index_html_AP);
    });

    // Route scan WiFi
    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
        int n = WiFi.scanNetworks();
        String json = "[";
        for (int i = 0; i < n; i++) {
            if (i > 0) json += ",";
            json += "{\"ssid\":\"" + WiFi.SSID(i) + "\","
                  + "\"rssi\":"    + WiFi.RSSI(i)  + ","
                  + "\"secure\":"  + (WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
        }
        json += "]";
        WiFi.scanDelete();
        request->send(200, "application/json", json);
    });

    // Route sauvegarde
    server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
        if (request->hasParam("ssid", true) && request->hasParam("pass", true) &&
            request->hasParam("webuser", true) && request->hasParam("webpass", true)) {

            String ssid    = request->getParam("ssid",    true)->value();
            String pass    = request->getParam("pass",    true)->value();
            String webuser = request->getParam("webuser", true)->value();
            String webpass = request->getParam("webpass", true)->value();

            saveWifiCredentials(ssid.c_str(), pass.c_str());
            saveAuthCredentials(webuser.c_str(), webpass.c_str());

            request->send(200);
            updateOLED("SAUVEGARDE", "Redemarrage...");

            #if DEBUG_MODE
                Serial.println("[AP] Credentials sauvegardés — reboot");
            #endif

            delay(1500);
            ESP.restart();
        } else {
            request->send(400);
        }
    });

    server.begin();
}