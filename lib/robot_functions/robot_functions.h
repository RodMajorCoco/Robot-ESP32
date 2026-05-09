#ifndef ROBOT_FUNCTIONS_H
#define ROBOT_FUNCTIONS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// config.h ou robot_functions.h
enum class Action {
    STOP,
    AVANCER,
    RECULER,
    ROTATION_G,
    ROTATION_D
};

// Variables globales partagées
extern unsigned long lastCommandTime;
extern String www_username;
extern String www_password;
extern Adafruit_SSD1306 display;
extern Preferences preferences;
extern bool isDisplayOn;
extern Action current_action;
extern SemaphoreHandle_t actionMutex;

// Fonctions utilitaires
void updateOLED(const String& line1, const String& line2);
bool isAuthenticated(AsyncWebServerRequest *request);
void saveWifiCredentials(const char* ssid, const char* pass);
void saveAuthCredentials(const char* user, const char* pass);
void setAction(Action action);
void applyMotorLogic(Action action);
void toggleDisplay(bool state);
const char* actionToString(Action a);

#endif