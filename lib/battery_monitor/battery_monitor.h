/************************************************************
 *  Fichier  : battery_monitor.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe BatteryMonitor — lecture ADC avec moyenne sur 16
 *    échantillons, conversion en tension via pont diviseur,
 *    calcul du pourcentage avec clamping.
 ************************************************************/

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "config.h"

// ---------------------------------------------------------------------------
class BatteryMonitor {
public:
    BatteryMonitor();

    /** Configure l'ADC. À appeler dans setup(). */
    void begin();

    /**
     * Lit la tension et met à jour le pourcentage interne.
     * À appeler périodiquement (ex. toutes les BATTERY_READ_INTERVAL ms).
     */
    void update();

    /** Retourne le dernier pourcentage calculé (0-100). */
    int getPercent() const { return _percent; }

private:
    volatile int _percent;
};

#endif // BATTERY_MONITOR_H