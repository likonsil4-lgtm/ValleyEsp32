#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include <Arduino.h>
#include "config.h"

class PositionTracker {
public:
    PositionTracker();
    void begin();
    void update(bool motorRunning, uint8_t direction);
    
    // ТОЛЬКО часы! Убраны методы с минутами чтобы не путаться
    void setCalibration(float startAngle, float rotationTimeHours);
    void resetToAngle(float angle);
    
    float getCurrentAngle();
    float getStartAngle();
    float getRotationTimeHours();  // Всегда возвращаем часы
    
private:
    float _currentAngle;
    float _startAngle;
    float _rotationTimeHours;  // ← Храним ТОЛЬКО в часах!
    unsigned long _lastUpdateTime;
    bool _wasRunning;
    uint8_t _lastValidDirection;
    
    float normalizeAngle(float angle);
};

#endif