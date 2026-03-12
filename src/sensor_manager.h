#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

struct SensorData {
    bool motorPowerPresent;
    uint8_t direction;        // DIR_UNKNOWN, DIR_CW, DIR_CCW
    float pressure;          
    unsigned long runtimeSeconds;
    bool motorRunning;
};

class SensorManager {
public:
    SensorManager();
    void begin();
    SensorData update();
    bool isMotorRunning();
    uint8_t getDirection();   // Возвращает DIR_UNKNOWN, DIR_CW или DIR_CCW
    float getPressure();
    
private:
    unsigned long _lastMotorHigh;
    unsigned long _motorStartTime;
    bool _motorRunning;
    uint8_t _direction;       // Текущее направление
    unsigned long _runtimeSeconds;
    
    void readMotorStatus();
    void readDirection();
    void readPressure(SensorData& data);
};

#endif