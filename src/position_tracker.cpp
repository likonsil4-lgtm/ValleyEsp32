#include "position_tracker.h"

PositionTracker::PositionTracker() {
    _currentAngle = 0.0;
    _startAngle = 220.0;
    _rotationTimeMinutes = 74.0;  // Оставляем в минутах для совместимости
    _lastUpdateTime = 0;
    _wasRunning = false;
}

void PositionTracker::begin() {
    _currentAngle = _startAngle;
    _lastUpdateTime = millis();
}

void PositionTracker::update(bool motorRunning, uint8_t direction) {
    if (!motorRunning || direction == DIR_UNKNOWN) {
        _lastUpdateTime = millis();
        _wasRunning = motorRunning;
        return;
    }
    
    unsigned long now = millis();
    
    if (!_wasRunning) {
        _lastUpdateTime = now;
        _wasRunning = true;
        return;
    }
    
    float deltaMinutes = (now - _lastUpdateTime) / 60000.0;
    _lastUpdateTime = now;
    
    // Расчет в минутах (как было)
    float degreesPerMinute = 360.0 / _rotationTimeMinutes;
    float angleChange = degreesPerMinute * deltaMinutes;
    
    if (direction == DIR_CW) {
        _currentAngle += angleChange;
        if (_currentAngle >= 360.0) {
            _currentAngle -= 360.0;
        }
    } else if (direction == DIR_CCW) {
        _currentAngle -= angleChange;
        if (_currentAngle < 0.0) {
            _currentAngle += 360.0;
        }
    }
}

void PositionTracker::setCalibration(float startAngle, float rotationTimeMinutes) {
    _startAngle = startAngle;
    _rotationTimeMinutes = rotationTimeMinutes;  // Принимаем минуты
    _currentAngle = startAngle;
}

// Добавляем метод для конвертации из часов
void PositionTracker::setCalibrationHours(float startAngle, float rotationTimeHours) {
    _startAngle = startAngle;
    _rotationTimeMinutes = rotationTimeHours * 60.0;  // Конвертируем часы в минуты
    _currentAngle = startAngle;
}

void PositionTracker::resetToAngle(float angle) {
    _currentAngle = normalizeAngle(angle);
}

float PositionTracker::getCurrentAngle() {
    return _currentAngle;
}

float PositionTracker::getStartAngle() {
    return _startAngle;
}

float PositionTracker::getRotationTime() {
    return _rotationTimeMinutes;  // Возвращаем минуты
}

float PositionTracker::getRotationTimeHours() {
    return _rotationTimeMinutes / 60.0;  // Конвертируем в часы для отображения
}

float PositionTracker::normalizeAngle(float angle) {
    while (angle >= 360.0) angle -= 360.0;
    while (angle < 0.0) angle += 360.0;
    return angle;
}