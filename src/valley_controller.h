#ifndef VALLEY_CONTROLLER_H
#define VALLEY_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

// Упрощённая state machine только для импульсов
enum MotorState {
    MOTOR_IDLE,
    PULSE_1_ACTIVE,      // Первый импульс (1 сек)
    PULSE_1_PAUSE,       // Пауза (2 сек)
    PULSE_2_ACTIVE,      // Второй импульс (1 сек)
    MOTOR_RUNNING        // Запущено, реле отключены
};

class ValleyController {
public:
    ValleyController();
    void begin();
    
    // Новые команды запуска
    void startCW();   // Пин 22: 1с ON, 2с OFF, 1с ON
    void startCCW();  // Пин 23: 1с ON, 2с OFF, 1с ON
    
    void stop();      // Как было: пин 19 на 3 секунды
    
    // Вызывать в loop() каждую итерацию!
    void update();
    
    bool isRunning();           // true если в процессе импульсов или запущено
    bool isInPulseSequence();   // true только во время импульсов
    uint8_t getActiveDirection(); // DIR_CW, DIR_CCW или DIR_UNKNOWN
    
private:
    MotorState _motorState;
    unsigned long _stateStartTime;
    uint8_t _activePin;         // Какой пин активен (22 или 23)
    uint8_t _pulseCount;        // Счётчик импульсов (1 или 2)
    
    void initRelays();
    void setRelay(int pin, bool state);
    void startPulseSequence(uint8_t pin);  // Общая логика для 22 и 23
    void abortPulseSequence();             // Экстренная остановка импульсов
    
    // Безопасность: гарантированное отключение обоих пинов направления
    void disableBothDirectionPins();
};

#endif