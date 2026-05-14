/************************************************************
 *  Fichier  : battery_monitor.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 ************************************************************/

#include "battery_monitor.h"

// ---------------------------------------------------------------------------
BatteryMonitor::BatteryMonitor()
    : _percent(100)
{}

// ---------------------------------------------------------------------------
void BatteryMonitor::begin() {
    pinMode(BATTERY_ADC_PIN, INPUT);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

// ---------------------------------------------------------------------------
void BatteryMonitor::update() {
    // Moyenne sur 16 lectures pour réduire le bruit ADC
    int raw = 0;
    for (int i = 0; i < 16; i++) {
        raw += analogRead(BATTERY_ADC_PIN);
    }
    raw /= 16;

    // Conversion en tension réelle
    float vADC = (raw / BATTERY_ADC_RES) * BATTERY_ADC_REF;
    float vBat = vADC * (BATTERY_R1 + BATTERY_R2) / BATTERY_R2;

    // Calcul du pourcentage avec clamping
    int pct = (int)(((vBat - BATTERY_VMIN) / (BATTERY_VMAX - BATTERY_VMIN)) * 100.0f);
    if (pct > 100) pct = 100;
    if (pct < 0)   pct = 0;

    _percent = pct;

#if DEBUG_MODE
    Serial.printf("[BAT] raw=%d vADC=%.3f vBat=%.3f pct=%d%%\n",
                  raw, vADC, vBat, pct);
#endif
}