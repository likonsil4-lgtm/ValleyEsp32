#include "position_tracker.h"

PositionTracker::PositionTracker() {
    _currentAngle = 0.0;
    _startAngle = 220.0;
    _rotationTimeMinutes = 74.0;
    _lastUpdateTime = 0;
    _wasRunning = false;
    _lastValidDirection = DIR_UNKNOWN;  // ← Теперь объявлено в .h
    _totalRotationTime = 0;               // ← Теперь объявлено в .h
    _fullRotations = 0;                   // ← Новое
    _absoluteAngle = 0.0;                 // ← Новое
}

void PositionTracker::begin() {
    _currentAngle = _startAngle;
    _lastUpdateTime = millis();
    _wasRunning = false;
    _lastValidDirection = DIR_UNKNOWN;
    _fullRotations = 0;
    _absoluteAngle = _startAngle;
}

void PositionTracker::update(bool motorRunning, uint8_t direction) {
    unsigned long now = millis();
    
    if (!motorRunning || direction == DIR_UNKNOWN) {
        if (_wasRunning) {
            Serial.print("Tracker: STOP at angle ");
            Serial.println(_currentAngle);
        }
        _wasRunning = false;
        _lastUpdateTime = now;
        return;
    }
    
    if (!_wasRunning) {
        _wasRunning = true;
        _lastValidDirection = direction;
        _lastUpdateTime = now;
        Serial.print("Tracker: START at angle ");
        Serial.print(_currentAngle);
        Serial.print(", direction ");
        Serial.println(direction == DIR_CW ? "CW" : "CCW");
        return;
    }
    
    if (direction != _lastValidDirection) {
        Serial.print("Tracker: Direction changed to ");
        Serial.println(direction == DIR_CW ? "CW" : "CCW");
        _lastValidDirection = direction;
    }
    
    unsigned long deltaMs = now - _lastUpdateTime;
    _lastUpdateTime = now;
    _totalRotationTime += deltaMs;
    
    if (deltaMs > 10000) {
        Serial.print("Tracker: Large time jump: ");
        Serial.print(deltaMs);
        Serial.println("ms, skipping");
        return;
    }
    
    float deltaHours = deltaMs / 3600000.0;
    float rotationTimeHours = _rotationTimeMinutes / 60.0;
    float degreesPerHour = 360.0 / rotationTimeHours;
    float angleChange = degreesPerHour * deltaHours;
    
    float previousAngle = _currentAngle;
    
    if (direction == DIR_CW) {
        _currentAngle += angleChange;
        _absoluteAngle += angleChange;
        
        if (_currentAngle >= 360.0) {
            _currentAngle -= 360.0;
            _fullRotations++;
            Serial.println("Tracker: 360° passed CW");
        }
    } else {
        _currentAngle -= angleChange;
        _absoluteAngle -= angleChange;
        
        if (_currentAngle < 0.0) {
            _currentAngle += 360.0;
            _fullRotations--;
            Serial.println("Tracker: 0° passed CCW");
        }
    }
}

void PositionTracker::setCalibration(float startAngle, float rotationTimeMinutes) {
    _startAngle = startAngle;
    _rotationTimeMinutes = rotationTimeMinutes;
    Serial.print("Calibration: start=");
    Serial.print(startAngle);
    Serial.print(", time=");
    Serial.print(rotationTimeMinutes);
    Serial.println("min");
}

void PositionTracker::setCalibrationHours(float startAngle, float rotationTimeHours) {
    setCalibration(startAngle, rotationTimeHours * 60.0);
}

void PositionTracker::resetToAngle(float angle) {
    _currentAngle = normalizeAngle(angle);
    _absoluteAngle = angle;  // Сбрасываем абсолютный тоже
    Serial.print("Tracker: Reset to ");
    Serial.println(_currentAngle);
}

float PositionTracker::getCurrentAngle() {
    return _currentAngle;
}

float PositionTracker::getStartAngle() {
    return _startAngle;
}

float PositionTracker::getRotationTime() {
    return _rotationTimeMinutes;
}

float PositionTracker::getRotationTimeHours() {
    return _rotationTimeMinutes / 60.0;
}

float PositionTracker::normalizeAngle(float angle) {
    while (angle >= 360.0) angle -= 360.0;
    while (angle < 0.0) angle += 360.0;
    return angle;
}