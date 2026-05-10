#ifndef CONFIG_H
#define CONFIG_H
#define BRAKE_MODE true

#include <Arduino.h>

// --- CONFIGURATION RÉSEAU ---
const int MAX_WIFI_RETRIES = 20;            // nombre de tentatives de connexion WiFi avant d'abandonner
const int MAX_PERIOD_WITHOUT_COMMAND = 5000; // Sécurité arrêt (ms)
const int WIFI_RECONNECT_INTERVAL = 10000;     // Intervalle de reconnexion WiFi (ms)
const char* const PREFS_NAMESPACE = "wifi-gate";

// --- CONFIGURATION ÉCRAN OLED ---
#define SDA_PIN 8 //
#define SCL_PIN 9 //
#define OLED_ADDR 0x3C

// --- CONFIGURATION MOTEURS (PINS PWM) ---
#define MOTEUR_A_IN1 4  // Moteur Gauche
#define MOTEUR_A_IN2 5
#define MOTEUR_B_IN1 6  // Moteur Droit
#define MOTEUR_B_IN2 7

// --- PARAMÈTRES DE MOUVEMENT ---
const int VITESSE_CROISIERE = 200; // Valeur entre 0 et 255
const int VITESSE_ROTATION = 180;



#endif