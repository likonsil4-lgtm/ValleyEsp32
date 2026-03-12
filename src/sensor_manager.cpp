#include "sensor_manager.h"

SensorManager::SensorManager() {
    _lastMotorHigh = 0;
    _motorStartTime = 0;
    _motorRunning = false;
    _direction = DIR_UNKNOWN;  // Изначально неизвестно!
    _runtimeSeconds = 0;
}

void SensorManager::begin() {
    pinMode(PIN_PRESSURE_SENSOR, INPUT);
    pinMode(PIN_MOTOR_STATUS, INPUT_PULLUP);
    pinMode(PIN_DIRECTION_CW, INPUT_PULLUP);
    pinMode(PIN_DIRECTION_CCW, INPUT_PULLUP);
    
    _lastMotorHigh = millis();
}

SensorData SensorManager::update() {
    SensorData data;
    
    readMotorStatus();
    readDirection();
    readPressure(data);
    
    data.motorRunning = _motorRunning;
    data.direction = _direction;  // Может быть DIR_UNKNOWN!
    data.runtimeSeconds = _runtimeSeconds;
    data.motorPowerPresent = (millis() - _lastMotorHigh < 1000);
    
    return data;
}

void SensorManager::readMotorStatus() {
    bool powerPresent = !digitalRead(PIN_MOTOR_STATUS);
    unsigned long now = millis();
    
    if (powerPresent) {
        _lastMotorHigh = now;
        if (!_motorRunning) {
            Serial.println("Motor START detected");
            _motorRunning = true;
            _motorStartTime = now;
            _runtimeSeconds = 0;
        }
    } else {
        if (_motorRunning && (now - _lastMotorHigh > MOTOR_STATUS_TIMEOUT_MS)) {
            Serial.println("Motor STOP detected");
            _motorRunning = false;
            _runtimeSeconds = 0;
            // Направление остается последним известным, не сбрасываем!
        }
    }
    
    if (_motorRunning) {
        _runtimeSeconds = (now - _motorStartTime) / 1000;
    }
}

void SensorManager::readDirection() {
    bool cwActive = !digitalRead(PIN_DIRECTION_CW);
    bool ccwActive = !digitalRead(PIN_DIRECTION_CCW);
    
    // Устанавливаем направление только когда есть питание на соответствующем пине
    if (cwActive) {
        _direction = DIR_CW;
        Serial.println("Direction detected: CW (Clockwise)");
    } else if (ccwActive) {
        _direction = DIR_CCW;
        Serial.println("Direction detected: CCW (Counter-Clockwise)");
    }
    // Если оба неактивны - оставляем как есть (последнее известное или UNKNOWN)
}

void SensorManager::readPressure(SensorData& data) {
    int rawValue = analogRead(PIN_PRESSURE_SENSOR);
    data.pressure = map(rawValue, 0, 4095, 0, 100);
}

bool SensorManager::isMotorRunning() {
    return _motorRunning;
}

uint8_t SensorManager::getDirection() {
    return _direction;
}

float SensorManager::getPressure() {
    int rawValue = analogRead(PIN_PRESSURE_SENSOR);
    return map(rawValue, 0, 4095, 0, 100);
}