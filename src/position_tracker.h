#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include <Arduino.h>
#include "config.h"

class PositionTracker {
public:
    PositionTracker();
    void begin();
    void update(bool motorRunning, uint8_t direction);
    void setCalibration(float startAngle, float rotationTimeMinutes);
    void setCalibrationHours(float startAngle, float rotationTimeHours);
    void resetToAngle(float angle);
    
    float getCurrentAngle();
    float getStartAngle();
    float getRotationTime();      // В минутах
    float getRotationTimeHours(); // В часах
    
    // НОВЫЕ методы:
    int getFullRotations() { return _fullRotations; }
    float getAbsoluteAngle() { return _absoluteAngle; }
    unsigned long getTotalRotationTimeMs() { return _totalRotationTime; }
    
private:
    float _currentAngle;
    float _startAngle;
    float _rotationTimeMinutes;
    unsigned long _lastUpdateTime;
    bool _wasRunning;
    
    // НОВЫЕ переменные (объявлены здесь!):
    uint8_t _lastValidDirection;
    unsigned long _totalRotationTime;
    int _fullRotations;
    float _absoluteAngle;
    
    float normalizeAngle(float angle);
};

#endif