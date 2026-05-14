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
    // Récupération conditionnelle du bus I2C :
    // on ne touche au bus que si SDA est réellement bloqué bas
    // (reboot en plein transfert I2C). Sur un boot normal, SDA est HIGH
    // et on n'envoie aucune impulsion qui perturberait le SSD1306 pendant
    // son démarrage interne (~100 ms après mise sous tension).
    pinMode(SDA_PIN, INPUT_PULLUP);
    delay(2);
    if (digitalRead(SDA_PIN) == LOW) {
        pinMode(SCL_PIN, OUTPUT);
        for (int i = 0; i < 9; i++) {
            digitalWrite(SCL_PIN, HIGH); delayMicroseconds(5);
            digitalWrite(SCL_PIN, LOW);  delayMicroseconds(5);
        }
        // Condition STOP
        pinMode(SDA_PIN, OUTPUT);
        digitalWrite(SDA_PIN, LOW);  delayMicroseconds(5);
        digitalWrite(SCL_PIN, HIGH); delayMicroseconds(5);
        digitalWrite(SDA_PIN, HIGH); delayMicroseconds(5);
        delay(5);
    }

    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(400000);   // 400 kHz → trame de 1024 o en ~25 ms au lieu de ~100 ms
    Wire.setTimeOut(50);     // 50 ms max par opération I2C : loop() ne se bloque plus jamais

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
void OledDisplay::applyPower(bool on) {
    _display.ssd1306_command(on ? SSD1306_DISPLAYON : SSD1306_DISPLAYOFF);
}
