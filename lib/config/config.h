/************************************************************
 *  Fichier  : config.h
 *  Projet   : Robot ESP32-S3
 *  Auteur   : 
 *  Date     : 2026-05-01
 *  Version  : 1.0
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
const char* const PREFS_NAMESPACE = "wifi-gate";

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



#endif