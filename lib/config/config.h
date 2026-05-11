/************************************************************
 *  Fichier  : config.h
 *  Projet   : Robot ESP32-S3
 *  Auteur   : 
 *  Date     : 2026-05-01
 *  Version  : 1.1
 *  Matériel : ESP32-S3 N16R8  + DRV8833 + SSD1306
 * ----------------------------------------------------------
 *  Description :
 *    Fichier de configuration centrale du projet.
 *    Regroupe les paramètres réseau, les pins matérielles,
 *    les vitesses de déplacement et les flags de compilation
 *    (DEBUG_MODE, BRAKE_MODE).
 * ----------------------------------------------------------
 *  Historique :
 *    1.0 - 2026-05-01 : Création
 *    1.1 - 2026-06-01 : Ajout de la configuration de la lecture de la batterie
 ************************************************************/



#ifndef CONFIG_H
#define CONFIG_H
#define BRAKE_MODE true
#define DEBUG_MODE true 

#include <Arduino.h>

// --- CONFIGURATION RÉSEAU ---
#define MAX_WIFI_RETRIES            20         // nombre de tentatives de connexion WiFi avant d'abandonner
#define MAX_PERIOD_WITHOUT_COMMAND  5000       // Sécurité arrêt (ms)
#define WIFI_RECONNECT_INTERVAL     10000     // Intervalle de reconnexion WiFi (ms)
#define PREFS_NAMESPACE "wifi-gate"

// --- MODE ACCESS POINT DE CONFIGURATION ---
#define AP_SSID             "Robot-Config"
#define AP_IP               "192.168.4.1"   // IP par défaut ESP32 en mode AP
#define AP_CONFIG_TIMEOUT   300000          // Timeout mode AP (ms) — 5 min, puis reboot



// --- CONFIGURATION ÉCRAN OLED ---
#define SDA_PIN     8
#define SCL_PIN     9
#define OLED_ADDR   0x3C

// --- CONFIGURATION MOTEURS (PINS PWM) ---
#define MOTEUR_A_IN1 4  // Moteur Gauche
#define MOTEUR_A_IN2 5
#define MOTEUR_B_IN1 6  // Moteur Droit
#define MOTEUR_B_IN2 7
#define DRV8833_EEP 10 // Pin de contrôle du mode de freinage du DRV8833 (EEPROM)

// --- PARAMÈTRES DE MOUVEMENT ---
#define VITESSE_CROISIERE         200
#define VITESSE_ROTATION          180

// --- MESURE TENSION BATTERIE ---
#define BATTERY_ADC_PIN     1           // GPIO ADC (éviter GPIO 0, 3 sur ESP32-S3)
#define BATTERY_R1          56000.0f   // Résistance haute du pont diviseur (Ω)
#define BATTERY_R2          56000.0f   // Résistance basse du pont diviseur (Ω)
#define BATTERY_VMAX        6.0f        // Tension max (4 piles neuves AA, V)
#define BATTERY_VMIN        4.0f        // Tension min (piles déchargées, V)
#define BATTERY_ADC_REF     3.3f        // Tension de référence ADC (V)
#define BATTERY_ADC_RES     4095.0f     // Résolution 12 bits
#define BATTERY_READ_INTERVAL 30000     // Intervalle de lecture (ms)



#endif