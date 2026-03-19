#ifndef VALLEY_CONTROLLER_H
#define VALLEY_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

enum MotorState {
  MOTOR_IDLE,
  MOTOR_STARTING_STEP1,  // Пин 21 включён, ждём 3с
  MOTOR_STARTING_STEP2,  // Направление включено, ждём 4с
  MOTOR_RUNNING          // Работает (после последовательности)
};

class ValleyController {
public:
    ValleyController();
    void begin();
    
    // Неблокирующий старт - возвращает управление сразу!
    void start(uint8_t direction);
    void stop();
    void changeDirection();
    
    // НОВОЕ: вызывать в loop() каждую итерацию!
    void update();
    
    bool isInStartingSequence();
    uint8_t getLastDirection();
    void setLastDirection(uint8_t dir);
    MotorState getState() { return _motorState; }
    
private:
    bool _mainRelayState;
    uint8_t _lastDirection;
    uint8_t _targetDirection;
    MotorState _motorState;
    unsigned long _stateStartTime;
    
    void initRelays();
    void setRelay(int pin, bool state);
};

#endif