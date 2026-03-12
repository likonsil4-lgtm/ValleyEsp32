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
    void setCalibrationHours(float startAngle, float rotationTimeHours);  // Новый метод
    void resetToAngle(float angle);
    
    float getCurrentAngle();
    float getStartAngle();
    float getRotationTime();      // В минутах
    float getRotationTimeHours(); // В часах (новый метод)
    
private:
    float _currentAngle;
    float _startAngle;
    float _rotationTimeMinutes;  // Храним в минутах
    unsigned long _lastUpdateTime;
    bool _wasRunning;
    
    float normalizeAngle(float angle);
};

#endif