/************************************************************
 *  Fichier  : config.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 *  Matériel : ESP32-S3 N16R8 + DRV8833 + SSD1306
 * ----------------------------------------------------------
 *  Description :
 *    Fichier de configuration centrale du projet.
 *    Regroupe les paramètres réseau, les pins matérielles,
 *    les vitesses de déplacement et les flags de compilation.
 ************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- FLAGS DE COMPILATION ---
#define BRAKE_MODE  true
#define DEBUG_MODE  true

// --- CONFIGURATION RÉSEAU ---
#define MAX_WIFI_RETRIES            20
#define MAX_PERIOD_WITHOUT_COMMAND  5000       // Sécurité arrêt (ms)
#define WIFI_RECONNECT_INTERVAL     10000      // Intervalle de reconnexion WiFi (ms)
#define PREFS_NAMESPACE             "wifi-gate"

// --- MODE ACCESS POINT DE CONFIGURATION ---
#define AP_SSID             "Robot-Config"
#define AP_IP               "192.168.4.1"
#define AP_CONFIG_TIMEOUT   300000             // Timeout mode AP (ms) — 5 min, puis reboot

// --- CONFIGURATION ÉCRAN OLED ---
#define SDA_PIN     8
#define SCL_PIN     9
#define OLED_ADDR   0x3C

// --- CONFIGURATION MOTEURS (PINS PWM) ---
#define MOTEUR_A_IN1  4   // Moteur Gauche
#define MOTEUR_A_IN2  5
#define MOTEUR_B_IN1  6   // Moteur Droit
#define MOTEUR_B_IN2  7
#define DRV8833_EEP   10  // Pin de contrôle du mode de freinage DRV8833

// --- PARAMÈTRES DE MOUVEMENT ---
#define VITESSE_CROISIERE   200
#define VITESSE_ROTATION    180

// --- MESURE TENSION BATTERIE ---
#define BATTERY_ADC_PIN       1
#define BATTERY_R1            56000.0f
#define BATTERY_R2            56000.0f
#define BATTERY_VMAX          6.0f
#define BATTERY_VMIN          4.0f
#define BATTERY_ADC_REF       3.3f
#define BATTERY_ADC_RES       4095.0f
#define BATTERY_READ_INTERVAL 30000            // Intervalle de lecture (ms)

#endif // CONFIG_H
