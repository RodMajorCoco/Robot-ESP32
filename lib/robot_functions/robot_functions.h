/************************************************************
 *  Fichier  : robot_functions.h
 *  Projet   : Robot ESP32-S3
 *  Auteur   : 
 *  Date     : 2026-05-01
 *  Version  : 1.1
 *  Matériel : ESP32-S3 + DRV8833 + SSD1306
 * ----------------------------------------------------------
 *  Description :
 *    Déclarations des fonctions de contrôle du robot,
 *    de l'enum Action, et des variables globales partagées
 *    entre main.cpp et robot_functions.cpp.
 * ----------------------------------------------------------
 *  Historique :
 *    1.0 - 2026-05-01 : Création
 *    1.1 - 2026-06-01 : Ajout de la lecture de la batterie
 ************************************************************/

#ifndef ROBOT_FUNCTIONS_H
#define ROBOT_FUNCTIONS_H


/************************************************************
 *              Gestion des includes                        *
 ***********************************************************/
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "config.h"
#include "web_interface.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/************************************************************
 *              Définition des actions                      *
 ***********************************************************/
enum class Action {
    STOP,
    AVANCER,
    RECULER,
    ROTATION_G,
    ROTATION_D
};

/************************************************************
 *              Variables globales partagées                *
 ***********************************************************/
extern unsigned long lastCommandTime;
extern String www_username;
extern String www_password;
extern Adafruit_SSD1306 display;
extern Preferences preferences;
extern volatile bool isDisplayOn;
extern volatile bool isDriverOn;
extern Action current_action;
extern SemaphoreHandle_t actionMutex;
extern volatile int32_t batteryPercent;
extern volatile bool isAPMode;

/************************************************************
 *                  Fonctions utilitaires                   *
 ***********************************************************/


/**
 * @brief Met à jour l'affichage OLED.
 *
 * Affiche deux lignes de texte sur l'écran OLED si l'écran est activé.
 *
 * @param line1 Première ligne affichée (titre)
 * @param line2 Deuxième ligne affichée (valeur)
 */
void updateOLED(const String& line1, const String& line2);


/**
 * @brief Vérifie l'authentification de la requête.
 *
 * Demande l'authentification HTTP si les identifiants ne correspondent pas.
 *
 * @param request Requête HTTP entrante
 * @return true si l'utilisateur est authentifié
 * @return false sinon
 */
bool isAuthenticated(AsyncWebServerRequest *request);

/**
 * @brief Sauvegarde les credentials WiFi.
 *
 * Enregistre le SSID et le mot de passe WiFi dans les Preferences.
 *
 * @param ssid Nom du réseau WiFi
 * @param pass Mot de passe du réseau WiFi
 */
void saveWifiCredentials(const char* ssid, const char* pass);


/**
 * @brief Sauvegarde les credentials Web.
 *
 * Enregistre le nom d'utilisateur et le mot de passe Web dans les Preferences.
 *
 * @param user Nom d'utilisateur Web
 * @param pass Mot de passe Web
 */
void saveAuthCredentials(const char* user, const char* pass);

/**
 * @brief Met à jour l'action courante du robot.
 *
 * Change l'action actuelle, met à jour l'heure de la dernière commande
 * et applique la logique moteurs correspondante.
 *
 * @param action Action à appliquer
 */
void setAction(Action action);

/**
 * @brief Applique la logique des moteurs selon l'action.
 *
 * Définit les sorties des moteurs pour avancer, reculer,
 * tourner ou arrêter le robot.
 *
 * @param action Action à traduire en commandes moteurs
 */
void applyMotorLogic(Action action);

/**
 * @brief Active ou désactive l'affichage OLED.
 *
 * Met à jour l'état local de l'écran OLED et rafraîchit l'affichage
 * lorsqu'on le rallume.
 *
 * @param state État souhaité de l'écran OLED
 */
void toggleDisplay(bool state);

/**
 * @brief Active ou désactive le driver moteur.
 * 
 * Met à jour l'état local du driver moteur et applique la logique moteur
 * correspondante (arrêt ou reprise de l'action courante).
 * 
 * @param state État souhaité du driver moteur
 */
void toggleDriver(bool state);

/**
 * @brief Convertit une action en texte lisible.
 *
 * Retourne la chaîne de caractères correspondant à l'action
 * pour l'affichage ou le débogage.
 *
 * @param a Action à convertir 
 * @return const char* Chaîne représentant l'action
 */
const char* actionToString(Action a);

/**
 * @brief Met à jour le pourcentage de batterie.
 *
 * Lit la tension de la batterie et calcule le pourcentage correspondant.
 */
void updateBattery();

/**
 * @brief Démarre le portail de configuration.
 *
 * Initialise et démarre le portail de configuration en mode AP.
 *
 * @param server Références au serveur web
 */
void startConfigPortal(AsyncWebServer& server);

#endif