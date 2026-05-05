#ifndef ROBOT_FUNCTIONS_H
#define ROBOT_FUNCTIONS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// Variables globales partagées
extern const char* current_action;
extern unsigned long lastCommandTime;
extern String www_username;
extern String www_password;
extern Adafruit_SSD1306 display;
extern Preferences preferences;

// Fonctions utilitaires
void updateOLED(String line1, String line2);
bool isAuthenticated(AsyncWebServerRequest *request);
void saveWifiCredentials(const char* ssid, const char* pass);
void saveAuthCredentials(const char* user, const char* pass);
void setAction(const char* action);

#endif