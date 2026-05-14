/************************************************************
 *  Fichier  : oled_display.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 ************************************************************/

#include "oled_display.h"

// ---------------------------------------------------------------------------
OledDisplay::OledDisplay()
    : _display(128, 64, &Wire, -1)
    , _isOn(true)
{}

// ---------------------------------------------------------------------------
void OledDisplay::begin() {
    Wire.begin(SDA_PIN, SCL_PIN);
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        for (;;); // halt — écran non détecté
    }
    _display.setTextColor(WHITE);
}

// ---------------------------------------------------------------------------
void OledDisplay::update(const String& line1, const String& line2, int batteryPct) {
    if (!_isOn) return;

    _display.clearDisplay();

    // --- Moitié haute : barre de batterie ---
    _display.drawRect(0, 0, 110, 14, WHITE);
    _display.fillRect(110, 4, 5, 6, WHITE); // borne +
    int fillWidth = (int)(106.0f * batteryPct / 100.0f);
    _display.fillRect(2, 2, fillWidth, 10, WHITE);
    _display.setTextSize(1);
    _display.setTextColor(batteryPct > 20 ? BLACK : WHITE);
    _display.setCursor(38, 3);
    _display.printf("%d%%", batteryPct);

    // --- Moitié basse : texte ---
    _display.setTextColor(WHITE);
    _display.setCursor(0, 20);
    _display.setTextSize(1);
    _display.println(line1);
    _display.setCursor(0, 32);
    _display.setTextSize(2);
    _display.println(line2);

    _display.display();
    yield();
}

// ---------------------------------------------------------------------------
void OledDisplay::toggle(bool state) {
    _isOn = state;
    _display.ssd1306_command(state ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
}
