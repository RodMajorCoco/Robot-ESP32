#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include <freertos/semphr.h>

#include "robot_functions.h"
#include "web_interface.h"
#include "config.h"


Adafruit_SSD1306 display(128, 64, &Wire, -1);

AsyncWebServer server(80);
Preferences preferences;

Action current_action = Action::STOP;
unsigned long lastCommandTime = 0;
String www_username;
String www_password;
volatile bool isDisplayOn = true;
volatile bool isDriverOn = true;

void setup() {
 
    // Configuration des pins moteurs
    pinMode(MOTEUR_A_IN1, OUTPUT);
    pinMode(MOTEUR_A_IN2, OUTPUT);
    pinMode(MOTEUR_B_IN1, OUTPUT);
    pinMode(MOTEUR_B_IN2, OUTPUT);   

    // Configuration du pin de contrôle du mode de freinage du DRV8833
    pinMode(DRV8833_EEP, OUTPUT);
    digitalWrite(DRV8833_EEP, HIGH);

    // Initialisation du mutex pour protéger l'accès à current_action
    actionMutex = xSemaphoreCreateMutex();

    // Initialisation de l'interface série pour le debug
    #if DEBUG_MODE
        Serial.begin(115200);
    #endif

    // Initialisation de l'écran OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) for(;;);
    
    display.setTextColor(WHITE);
    updateOLED("ROBOT S3 READY", "BOOTING...");

    // Récupération WiFi via Preferences
    preferences.begin(PREFS_NAMESPACE, true);
    String ssid = preferences.getString("ssid", "ERR");
    String pass = preferences.getString("password", "ERR");
    preferences.end();

    // Connexion au WiFi
    WiFi.begin(ssid.c_str(), pass.c_str());
    int timeout_counter = 0;
    while (WiFi.status() != WL_CONNECTED && timeout_counter < MAX_WIFI_RETRIES) {
        delay(500);
        timeout_counter++;
        updateOLED("ROBOT S3 READY", "Tentative: " + String(timeout_counter));
    }

    if (WiFi.status() == WL_CONNECTED) {
        updateOLED("CONNECTE", WiFi.localIP().toString());
    } else {
        updateOLED("ERREUR WIFI", "Verif config");
    }

    // Récupération des credentials Web via Preferences
    preferences.begin(PREFS_NAMESPACE, true);
    www_username = preferences.getString("web_user", "ERR");
    www_password = preferences.getString("web_pass", "ERR");
    preferences.end();

    // Configuration du serveur web pour les différentes actions
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;
        request->send_P(200, "text/html", index_html);
    });

    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] forward");
        #endif

        setAction(Action::AVANCER);
        request->send(200);
    });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] backward");
        #endif

        setAction(Action::RECULER);
        request->send(200);
    });

    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] left");
        #endif

        setAction(Action::ROTATION_G);
        request->send(200);
    });

    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] right");
        #endif

        setAction(Action::ROTATION_D);
        request->send(200);
    });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] stop");
        #endif

        setAction(Action::STOP);
        request->send(200);
    });

    server.on("/display/on", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] display on");
        #endif

        toggleDisplay(true);
        request->send(200);
    });

    server.on("/display/off", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] display off");
        #endif

        toggleDisplay(false);
        request->send(200);
    });

    server.on("/driver/on", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;
        
        #if DEBUG_MODE
            Serial.println("[CMD] driver on");
        #endif

        toggleDriver(true);
        request->send(200);
    });

    server.on("/driver/off", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request)) return;

        #if DEBUG_MODE
            Serial.println("[CMD] driver off");
        #endif

        toggleDriver(false);
        request->send(200);
    });

    server.begin();

    lastCommandTime = millis(); // Initialisation du timer de sécurité
}

void loop() {

    // Si la connexion est perdue, on tente de se reconnecter toutes les 10 secondes
    static unsigned long lastWifiCheck = 0;
    if (WiFi.status() != WL_CONNECTED && (millis() - lastWifiCheck > WIFI_RECONNECT_INTERVAL)) {
        
        #if DEBUG_MODE
            Serial.println("WiFi perdu, reconnexion...");
        #endif
        
        updateOLED("ERREUR WIFI", "reconnexion...");
        WiFi.reconnect();
        lastWifiCheck = millis();
    }

    // Si aucune commande n'est reçue depuis plus de 5 secondes, on remet l'état à STOP
    Action snapshotAction;
    unsigned long snapshotActionTime;

    if (xSemaphoreTake(actionMutex, portMAX_DELAY)) {
        snapshotAction = current_action;
        snapshotActionTime = lastCommandTime;
        xSemaphoreGive(actionMutex);
    }

    if(snapshotAction != Action::STOP && (millis() - snapshotActionTime > MAX_PERIOD_WITHOUT_COMMAND)) {
        setAction(Action::STOP);
    }

   
}