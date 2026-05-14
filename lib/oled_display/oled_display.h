/************************************************************
 *  Fichier  : oled_display.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe OledDisplay — encapsule l'écran SSD1306 128×64.
 *    Affiche une barre de batterie en moitié haute et
 *    deux lignes de texte en moitié basse.
 ************************************************************/

#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// ---------------------------------------------------------------------------
class OledDisplay {
public:
    OledDisplay();

    /**
     * Initialise le bus I2C et l'écran.
     * Bloque indéfiniment si l'écran n'est pas détecté.
     */
    void begin();

    /**
     * Rafraîchit l'affichage.
     * @param line1  Titre (petite police)
     * @param line2  Valeur / état (grande police)
     * @param batteryPct  Pourcentage batterie (0-100)
     */
    void update(const String& line1, const String& line2, int batteryPct);

    /**
     * Marque l'écran comme actif/inactif (état seul, sans I2C).
     * Appelable depuis n'importe quelle tâche.
     * La commande DISPLAYON/DISPLAYOFF est envoyée par applyPower()
     * depuis loop() (main task).
     */
    void toggle(bool state) { _isOn = state; }

    /** Envoie la commande DISPLAYON/DISPLAYOFF via I2C. À appeler depuis loop(). */
    void applyPower(bool on);

    bool isOn() const { return _isOn; }

private:
    Adafruit_SSD1306 _display;
    volatile bool    _isOn;
};

#endif // OLED_DISPLAY_H
