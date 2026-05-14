/************************************************************
 *  Fichier  : battery_monitor.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe BatteryMonitor — lecture ADC avec moyenne sur 16
 *    échantillons, conversion en tension via pont diviseur
 *    (R1 = R2 = 56 kΩ), calcul du pourcentage avec clamping
 *    entre BATTERY_VMIN et BATTERY_VMAX.
 ************************************************************/

#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>
#include "config.h"

// ---------------------------------------------------------------------------
class BatteryMonitor {
public:
    BatteryMonitor();

    /**
     * Configure la broche ADC et la résolution (12 bits, 11 dB).
     * À appeler une seule fois dans setup().
     */
    void begin();

    /**
     * Effectue 16 lectures ADC, calcule la tension batterie via le
     * pont diviseur et met à jour le pourcentage interne.
     * À appeler périodiquement depuis loop() (ex. toutes les
     * BATTERY_READ_INTERVAL ms). Non thread-safe.
     */
    void update();

    /**
     * Retourne le dernier pourcentage calculé par update().
     * @return  Pourcentage batterie clampé entre 0 et 100.
     */
    int getPercent() const { return _percent; }

private:
    volatile int _percent;
};

#endif // BATTERY_MONITOR_H
