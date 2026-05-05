#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

#include "robot_functions.h"
#include "web_interface.h"

// OLED sur Pins 8 et 9
#define SDA_PIN 8
#define SCL_PIN 9
Adafruit_SSD1306 display(128, 64, &Wire, -1);

AsyncWebServer server(80);
Preferences preferences;

const char* current_action = "STOP";
unsigned long lastCommandTime = 0;
String www_username;
String www_password;

const int MAX_WIFI_RETRIES = 20;
const int MAX_PERIOD_WITHOUT_COMMAND = 5000;


void setup() {

    

    // Initialisation de l'interface série pour le debug
    Serial.begin(115200);

    // Initialisation de l'écran OLED
    Wire.begin(SDA_PIN, SCL_PIN);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
    
    display.setTextColor(WHITE);
    updateOLED("ROBOT S3 READY", "BOOTING...");

    // Récupération WiFi via Preferences
    preferences.begin("wifi-gate", true);
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
    preferences.begin("wifi-gate", true);
    www_username = preferences.getString("web_user", "ERR");
    www_password = preferences.getString("web_pass", "ERR");
    preferences.end();

    // Configuration du serveur web pour les différentes actions
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        request->send_P(200, "text/html", index_html);
    });

    server.on("/forward", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        setAction("AVANCER");
        request->send(200);
    });

    server.on("/backward", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        setAction("RECULER");
        request->send(200);
    });

    server.on("/left", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        setAction("ROTATION G");
        request->send(200);
    });

    server.on("/right", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        setAction("ROTATION D");
        request->send(200);
    });

    server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!isAuthenticated(request))
            return;
        setAction("STOP");
        request->send(200);
    });

    server.begin();
}

void loop() {

    // Si la connexion est perdue, on tente de se reconnecter toutes les 10 secondes
    static unsigned long lastWifiCheck = 0;
    if (WiFi.status() != WL_CONNECTED && (millis() - lastWifiCheck > 10000)) {
        Serial.println("WiFi perdu, reconnexion...");
        updateOLED("ERREUR WIFI", "reconnexion...");
        WiFi.reconnect();
        lastWifiCheck = millis();
    }

    // Si aucune commande n'est reçue depuis plus de 5 secondes, on remet l'état à STOP
    if(current_action != "STOP" && (millis() - lastCommandTime > MAX_PERIOD_WITHOUT_COMMAND)) {
        setAction("STOP");
    }



}