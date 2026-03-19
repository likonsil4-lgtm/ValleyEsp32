#include "position_tracker.h"

PositionTracker::PositionTracker() {
    _currentAngle = 0.0;
    _startAngle = 220.0;
    _rotationTimeHours = 74.0;  // ← По умолчанию 74 часа (не минуты!)
    _lastUpdateTime = 0;
    _wasRunning = false;
    _lastValidDirection = DIR_UNKNOWN;
}

void PositionTracker::begin() {
    _currentAngle = _startAngle;
    _lastUpdateTime = millis();
    _wasRunning = false;
    _lastValidDirection = DIR_UNKNOWN;
    
    Serial.print("Tracker begin: startAngle=");
    Serial.print(_startAngle);
    Serial.print(", rotationTime=");
    Serial.print(_rotationTimeHours);
    Serial.println(" hours");
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
    
    // Мотор только что запустился
    if (!_wasRunning) {
        _wasRunning = true;
        _lastValidDirection = direction;
        _lastUpdateTime = now;
        Serial.print("Tracker: START at angle ");
        Serial.print(_currentAngle);
        Serial.print(", direction ");
        Serial.print(direction == DIR_CW ? "CW" : "CCW");
        Serial.print(", rotationTime=");
        Serial.print(_rotationTimeHours);
        Serial.println("h");
        return;
    }
    
    // Направление поменялось
    if (direction != _lastValidDirection) {
        Serial.print("Tracker: Direction changed to ");
        Serial.println(direction == DIR_CW ? "CW" : "CCW");
        _lastValidDirection = direction;
    }
    
    // ═══════════════════════════════════════════════════════════
    // РАСЧЁТ УГЛА - только в часах!
    // ═══════════════════════════════════════════════════════════
    unsigned long deltaMs = now - _lastUpdateTime;
    _lastUpdateTime = now;
    
    // Защита от скачков времени
    if (deltaMs > 30000) {  // Пропускаем если > 30 секунд
        Serial.print("Tracker: Time jump ");
        Serial.print(deltaMs);
        Serial.println("ms, skipping");
        return;
    }
    
    // Скорость: 360 градусов / rotationTimeHours
    // Пример: 74 часа на оборот = 360/74 = 4.86 градуса/час
    float degreesPerHour = 360.0 / _rotationTimeHours;
    
    // Время в часах (миллисекунды → часы)
    float deltaHours = deltaMs / 3600000.0;  // 1 час = 3,600,000 мс
    
    // Изменение угла
    float angleChange = degreesPerHour * deltaHours;
    
    // Применяем
    if (direction == DIR_CW) {
        _currentAngle += angleChange;
        if (_currentAngle >= 360.0) {
            _currentAngle -= 360.0;
            Serial.println("Tracker: 360° passed CW");
        }
    } else {  // CCW
        _currentAngle -= angleChange;
        if (_currentAngle < 0.0) {
            _currentAngle += 360.0;
            Serial.println("Tracker: 0° passed CCW");
        }
    }
}

// ═══════════════════════════════════════════════════════════
// КАЛИБРОВКА - принимаем только часы!
// ═══════════════════════════════════════════════════════════
void PositionTracker::setCalibration(float startAngle, float rotationTimeHours) {
    _startAngle = startAngle;
    _rotationTimeHours = rotationTimeHours;  // ← Часы, без конвертации!
    _currentAngle = startAngle;  // ← Сразу сбрасываем в стартовый угол!
    
    Serial.print("Calibration: startAngle=");
    Serial.print(_startAngle);
    Serial.print("°, rotationTime=");
    Serial.print(_rotationTimeHours);
    Serial.println(" hours");
}

void PositionTracker::resetToAngle(float angle) {
    _currentAngle = normalizeAngle(angle);
    Serial.print("Tracker: Reset to ");
    Serial.println(_currentAngle);
}

float PositionTracker::getCurrentAngle() {
    return _currentAngle;
}

float PositionTracker::getStartAngle() {
    return _startAngle;
}

float PositionTracker::getRotationTimeHours() {
    return _rotationTimeHours;  // ← Возвращаем часы
}

float PositionTracker::normalizeAngle(float angle) {
    while (angle >= 360.0) angle -= 360.0;
    while (angle < 0.0) angle += 360.0;
    return angle;
}