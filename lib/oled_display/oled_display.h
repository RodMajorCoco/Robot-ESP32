/************************************************************
 *  Fichier  : oled_display.h
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 * ----------------------------------------------------------
 *  Description :
 *    Classe OledDisplay — encapsule l'écran SSD1306 128×64.
 *
 *    Séparation stricte des responsabilités :
 *      • toggle() → mise à jour d'état uniquement, appelable
 *        depuis n'importe quelle tâche (pas d'I2C).
 *      • update() / applyPower() → accès I2C/Wire, à appeler
 *        exclusivement depuis loop() sur le core 1.
 *
 *    Layout de l'écran :
 *      • Moitié haute (y 0-13)  : barre de batterie graphique.
 *      • Moitié basse (y 20-63) : line1 (petite police) et
 *                                  line2 (grande police).
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
     * Initialise le bus I2C et l'écran SSD1306.
     * Effectue une récupération conditionnelle du bus si SDA est
     * bloqué bas (reboot en plein transfert I2C). Bloque
     * indéfiniment si l'écran n'est pas détecté.
     * À appeler une seule fois dans setup().
     */
    void begin();

    /**
     * Efface l'écran et redessine la barre de batterie et les
     * deux lignes de texte. À appeler depuis loop() uniquement
     * (accès Wire/I2C — core 1 requis).
     * @param line1       Titre affiché en petite police (y=20).
     * @param line2       État affiché en grande police (y=32).
     * @param batteryPct  Pourcentage batterie (0–100).
     */
    void update(const String& line1, const String& line2, int batteryPct);

    /**
     * Met à jour le flag d'état de l'écran sans toucher au bus I2C.
     * Appelable depuis n'importe quelle tâche (callback web, ISR…).
     * La commande DISPLAYON/DISPLAYOFF est envoyée par applyPower()
     * lors du prochain cycle de loop().
     * @param state  true = écran actif, false = écran éteint.
     */
    void toggle(bool state) { _isOn = state; }

    /**
     * Envoie la commande DISPLAYON ou DISPLAYOFF via I2C.
     * À appeler exclusivement depuis loop() (core 1).
     * @param on  true = allumer l'écran, false = éteindre.
     */
    void applyPower(bool on);

    /**
     * Retourne l'état courant de l'écran.
     * @return  true si l'écran est marqué actif, false sinon.
     */
    bool isOn() const { return _isOn; }

private:
    Adafruit_SSD1306 _display;
    volatile bool    _isOn;
};

#endif // OLED_DISPLAY_H
