/************************************************************
 *  Fichier  : motor_controller.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe MotorController — commande PWM des deux moteurs
 *    via DRV8833, avec mutex FreeRTOS sur l'état partagé.
 *
 *    Séparation stricte des responsabilités :
 *      • setAction() / toggleDriver() / setVitesse*() → mise à
 *        jour d'état uniquement, appelables depuis n'importe
 *        quelle tâche (thread-safe).
 *      • applyMotorLogic() → écriture PWM (LEDC), à appeler
 *        exclusivement depuis loop() sur le core 1.
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

    /**
     * Initialise les GPIO moteurs, le mutex FreeRTOS et applique
     * STOP sur les pins PWM. À appeler une seule fois dans setup().
     */
    void begin();

    /**
     * Mémorise la nouvelle action et réinitialise le timer watchdog.
     * Thread-safe (mutex FreeRTOS). N'écrit PAS les PWM — délégué
     * à applyMotorLogic() appelé depuis loop().
     * @param action  Nouvelle action à exécuter.
     */
    void setAction(Action action);

    /**
     * Écrit les niveaux PWM sur les pins moteur selon l'action.
     * À appeler exclusivement depuis loop() (core 1) : l'API LEDC
     * de l'Arduino core 3.x n'est pas thread-safe sur l'ESP32-S3.
     * @param action  Action dont on applique la logique PWM.
     */
    void applyMotorLogic(Action action);

    /**
     * Active ou désactive le driver DRV8833 via sa pin EEP.
     * Si désactivé, remet l'action courante à STOP (thread-safe).
     * @param state  true = driver actif, false = driver en veille.
     */
    void toggleDriver(bool state);

    /**
     * Retourne l'action courante (thread-safe via mutex).
     * @return  Dernière action enregistrée par setAction().
     */
    Action getCurrentAction();

    /**
     * Retourne l'horodatage (ms) de la dernière commande reçue.
     * Thread-safe. Utilisé par le watchdog de sécurité dans loop().
     * @return  Valeur de millis() au moment du dernier setAction().
     */
    unsigned long getLastCommandTime();

    /**
     * Retourne true si le driver DRV8833 est actif.
     * Non thread-safe — lire uniquement depuis loop().
     */
    bool isDriverEnabled() const { return _driverOn; }

    /**
     * Règle la vitesse de croisière (avancer / reculer).
     * Thread-safe (écriture atomique 32 bits). Positionne le flag
     * interne pour que loop() ré-applique le PWM immédiatement.
     * @param v  Valeur PWM clampée entre 0 et 255.
     */
    void setVitesseCroisiere(int v);

    /**
     * Règle la vitesse de rotation (gauche / droite).
     * Thread-safe (écriture atomique 32 bits). Positionne le flag
     * interne pour que loop() ré-applique le PWM immédiatement.
     * @param v  Valeur PWM clampée entre 0 et 255.
     */
    void setVitesseRotation(int v);

    /** Retourne la vitesse de croisière courante (PWM 0-255). */
    int getVitesseCroisiere() const { return _vitesseCroisiere; }

    /** Retourne la vitesse de rotation courante (PWM 0-255). */
    int getVitesseRotation()  const { return _vitesseRotation;  }

    /**
     * Retourne true si une vitesse a changé depuis le dernier appel,
     * et remet le flag à false. Appelé depuis loop() pour déclencher
     * un ré-applique PWM sans attendre la prochaine commande.
     */
    bool consumeSpeedChanged();

    /**
     * Convertit une valeur Action en chaîne de caractères lisible.
     * @param  a  Action à convertir.
     * @return    Pointeur vers une chaîne littérale constante.
     */
    static const char* actionToString(Action a);

private:
    volatile bool       _driverOn;
    Action              _currentAction;
    unsigned long       _lastCommandTime;
    SemaphoreHandle_t   _mutex;
    volatile int        _vitesseCroisiere;
    volatile int        _vitesseRotation;
    volatile bool       _speedChanged;
};

#endif // MOTOR_CONTROLLER_H
