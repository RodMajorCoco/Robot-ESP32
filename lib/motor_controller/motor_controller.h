/************************************************************
 *  Fichier  : motor_controller.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe MotorController — commande PWM des deux moteurs
 *    via DRV8833, avec mutex FreeRTOS sur l'action courante
 *    et watchdog de sécurité (arrêt auto si plus de commande).
 ************************************************************/

#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

// ---------------------------------------------------------------------------
// Enum Action — partagé par l'ensemble du projet
// ---------------------------------------------------------------------------
enum class Action {
    STOP,
    AVANCER,
    RECULER,
    ROTATION_G,
    ROTATION_D
};

// ---------------------------------------------------------------------------
class MotorController {
public:
    MotorController();

    /** Initialise les GPIO et le mutex. À appeler dans setup(). */
    void begin();

    /**
     * Change l'action courante, réinitialise le timer watchdog
     * et applique immédiatement la logique moteur.
     */
    void setAction(Action action);

    /**
     * Applique directement la logique moteur sans toucher à
     * l'action courante (utilisé par le watchdog et toggleDriver).
     */
    void applyMotorLogic(Action action);

    /** Active ou désactive le driver DRV8833. */
    void toggleDriver(bool state);

    /** Retourne l'action courante (thread-safe). */
    Action getCurrentAction();

    /** Retourne l'horodatage de la dernière commande (thread-safe). */
    unsigned long getLastCommandTime();

    /** Retourne true si le driver est actif. */
    bool isDriverEnabled() const { return _driverOn; }

    /** Convertit une Action en chaîne lisible. */
    static const char* actionToString(Action a);

private:
    volatile bool       _driverOn;
    Action              _currentAction;
    unsigned long       _lastCommandTime;
    SemaphoreHandle_t   _mutex;
};

#endif // MOTOR_CONTROLLER_H
