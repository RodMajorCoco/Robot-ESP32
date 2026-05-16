/************************************************************
 *  Fichier  : motor_controller.cpp
 *  Projet   : Robot ESP32-S3
 *  Version  : 2.0 — refactoring OO
 ************************************************************/

#include "motor_controller.h"

// ---------------------------------------------------------------------------
MotorController::MotorController()
    : _driverOn(true)
    , _currentAction(Action::STOP)
    , _lastCommandTime(0)
    , _mutex(nullptr)
    , _vitesseCroisiere(constrain(VITESSE_CROISIERE, 0, 255))
    , _vitesseRotation(constrain(VITESSE_ROTATION,  0, 255))
    , _speedChanged(false)
{}

// ---------------------------------------------------------------------------
void MotorController::begin() {
    pinMode(MOTEUR_A_IN1, OUTPUT);
    pinMode(MOTEUR_A_IN2, OUTPUT);
    pinMode(MOTEUR_B_IN1, OUTPUT);
    pinMode(MOTEUR_B_IN2, OUTPUT);

    pinMode(DRV8833_EEP, OUTPUT);
    digitalWrite(DRV8833_EEP, HIGH); // driver actif par défaut

    _mutex = xSemaphoreCreateMutex();
    _lastCommandTime = millis();
    applyMotorLogic(Action::STOP);
}

// ---------------------------------------------------------------------------
void MotorController::setAction(Action action) {
    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        _currentAction   = action;
        _lastCommandTime = millis();
        xSemaphoreGive(_mutex);
    }
    // applyMotorLogic est appelé uniquement depuis loop() (main task)
}

// ---------------------------------------------------------------------------
void MotorController::applyMotorLogic(Action action) {

#if DEBUG_MODE
    Serial.print("[MOTOR] ");
    Serial.println(actionToString(action));
#endif

    if (!_driverOn && action != Action::STOP) {
        return; // driver OFF : on n'envoie rien sauf STOP
    }

    switch (action) {
        case Action::AVANCER:
            analogWrite(MOTEUR_A_IN1, _vitesseCroisiere);
            analogWrite(MOTEUR_A_IN2, 0);
            analogWrite(MOTEUR_B_IN1, _vitesseCroisiere);
            analogWrite(MOTEUR_B_IN2, 0);
            break;

        case Action::RECULER:
            analogWrite(MOTEUR_A_IN1, 0);
            analogWrite(MOTEUR_A_IN2, _vitesseCroisiere);
            analogWrite(MOTEUR_B_IN1, 0);
            analogWrite(MOTEUR_B_IN2, _vitesseCroisiere);
            break;

        case Action::ROTATION_G:
            analogWrite(MOTEUR_A_IN1, 0);
            analogWrite(MOTEUR_A_IN2, _vitesseRotation);
            analogWrite(MOTEUR_B_IN1, _vitesseRotation);
            analogWrite(MOTEUR_B_IN2, 0);
            break;

        case Action::ROTATION_D:
            analogWrite(MOTEUR_A_IN1, _vitesseRotation);
            analogWrite(MOTEUR_A_IN2, 0);
            analogWrite(MOTEUR_B_IN1, 0);
            analogWrite(MOTEUR_B_IN2, _vitesseRotation);
            break;

        case Action::STOP:
        default:
#if BRAKE_MODE
            // Freinage actif : les deux pins HIGH par moteur
            analogWrite(MOTEUR_A_IN1, 255);
            analogWrite(MOTEUR_A_IN2, 255);
            analogWrite(MOTEUR_B_IN1, 255);
            analogWrite(MOTEUR_B_IN2, 255);
#else
            // Roues libres : les deux pins LOW par moteur
            analogWrite(MOTEUR_A_IN1, 0);
            analogWrite(MOTEUR_A_IN2, 0);
            analogWrite(MOTEUR_B_IN1, 0);
            analogWrite(MOTEUR_B_IN2, 0);
#endif
            break;
    }
}

// ---------------------------------------------------------------------------
void MotorController::toggleDriver(bool state) {
    _driverOn = state;
    digitalWrite(DRV8833_EEP, state ? HIGH : LOW);

    if (!state) {
        if (xSemaphoreTake(_mutex, pdMS_TO_TICKS(10))) {
            _currentAction   = Action::STOP;
            _lastCommandTime = millis();
            xSemaphoreGive(_mutex);
        }
        // applyMotorLogic(STOP) sera appelé depuis loop() lors du prochain tick
    }
}

// ---------------------------------------------------------------------------
void MotorController::setVitesseCroisiere(int v) {
    _vitesseCroisiere = constrain(v, 0, 255);
    _speedChanged = true;
}

// ---------------------------------------------------------------------------
void MotorController::setVitesseRotation(int v) {
    _vitesseRotation = constrain(v, 0, 255);
    _speedChanged = true;
}

// ---------------------------------------------------------------------------
bool MotorController::consumeSpeedChanged() {
    if (!_speedChanged) return false;
    _speedChanged = false;
    return true;
}

// ---------------------------------------------------------------------------
Action MotorController::getCurrentAction() {
    Action snap = Action::STOP;
    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        snap = _currentAction;
        xSemaphoreGive(_mutex);
    }
    return snap;
}

// ---------------------------------------------------------------------------
unsigned long MotorController::getLastCommandTime() {
    unsigned long t = 0;
    if (xSemaphoreTake(_mutex, portMAX_DELAY)) {
        t = _lastCommandTime;
        xSemaphoreGive(_mutex);
    }
    return t;
}

// ---------------------------------------------------------------------------
const char* MotorController::actionToString(Action a) {
    switch (a) {
        case Action::AVANCER:    return "AVANCER";
        case Action::RECULER:    return "RECULER";
        case Action::ROTATION_G: return "ROTATION G";
        case Action::ROTATION_D: return "ROTATION D";
        default:                 return "STOP";
    }
}
