#include "valley_controller.h"

ValleyController::ValleyController() {
    _motorState = MOTOR_IDLE;
    _stateStartTime = 0;
    _activePin = 0;
    _pulseCount = 0;
}

void ValleyController::begin() {
    initRelays();
}

void ValleyController::initRelays() {
    pinMode(PIN_RELAY_MAIN, OUTPUT);
    pinMode(PIN_RELAY_START, OUTPUT);  // Больше не используется, но оставим для совместимости
    pinMode(PIN_RELAY_DIR_CW, OUTPUT);
    pinMode(PIN_RELAY_DIR_CCW, OUTPUT);

    // Все выключены (HIGH для active LOW)
    digitalWrite(PIN_RELAY_MAIN, HIGH);
    digitalWrite(PIN_RELAY_START, HIGH);
    digitalWrite(PIN_RELAY_DIR_CW, HIGH);
    digitalWrite(PIN_RELAY_DIR_CCW, HIGH);

    _motorState = MOTOR_IDLE;
    _activePin = 0;
}

// Безопасное отключение обоих пинов направления
void ValleyController::disableBothDirectionPins() {
    digitalWrite(PIN_RELAY_DIR_CW, HIGH);   // HIGH = OFF для active LOW
    digitalWrite(PIN_RELAY_DIR_CCW, HIGH);  // HIGH = OFF
    Serial.println("Safety: Both direction pins disabled");
}

// Запуск по часовой (пин 22)
void ValleyController::startCW() {
    if (_motorState != MOTOR_IDLE) {
        Serial.println("Valley: STARTCW rejected - already active");
        return;
    }
    
    Serial.println("Valley: STARTCW sequence begin");
    startPulseSequence(PIN_RELAY_DIR_CW);
}

// Запуск против часовой (пин 23)
void ValleyController::startCCW() {
    if (_motorState != MOTOR_IDLE) {
        Serial.println("Valley: STARTCCW rejected - already active");
        return;
    }
    
    Serial.println("Valley: STARTCCW sequence begin");
    startPulseSequence(PIN_RELAY_DIR_CCW);
}

// Общая логика импульсов
void ValleyController::startPulseSequence(uint8_t pin) {
    // Гарантированная безопасность: выключаем оба перед стартом
    disableBothDirectionPins();
    
    _activePin = pin;
    _pulseCount = 1;
    _motorState = PULSE_1_ACTIVE;
    _stateStartTime = millis();
    
    // Первый импульс
    setRelay(pin, true);
    Serial.print("Pulse 1: Pin ");
    Serial.print(pin);
    Serial.println(" ON");
}

// Экстренная остановка импульсов
void ValleyController::abortPulseSequence() {
    if (_activePin != 0) {
        setRelay(_activePin, false);
    }
    disableBothDirectionPins();
    _motorState = MOTOR_IDLE;
    _activePin = 0;
    _pulseCount = 0;
    Serial.println("Pulse sequence aborted");
}

// Неблокирующий update - вызывать в loop!
void ValleyController::update() {
    if (_motorState == MOTOR_IDLE) return;
    
    unsigned long now = millis();
    unsigned long elapsed = now - _stateStartTime;
    
    switch (_motorState) {
        case PULSE_1_ACTIVE:
            // Первый импульс: 1 секунда
            if (elapsed >= PULSE_DURATION_MS) {
                setRelay(_activePin, false);  // Выключаем
                _motorState = PULSE_1_PAUSE;
                _stateStartTime = now;
                Serial.println("Pulse 1: OFF, starting 2s pause");
            }
            break;
            
        case PULSE_1_PAUSE:
            // Пауза: 2 секунды
            if (elapsed >= PULSE_PAUSE_MS) {
                setRelay(_activePin, true);   // Включаем снова
                _motorState = PULSE_2_ACTIVE;
                _stateStartTime = now;
                _pulseCount = 2;
                Serial.print("Pulse 2: Pin ");
                Serial.print(_activePin);
                Serial.println(" ON");
            }
            break;
            
        case PULSE_2_ACTIVE:
            // Второй импульс: 1 секунда
            if (elapsed >= PULSE_DURATION_MS) {
                setRelay(_activePin, false);  // Выключаем
                _motorState = MOTOR_RUNNING;  // Или MOTOR_IDLE если не отслеживаем
                _stateStartTime = 0;
                Serial.println("Pulse 2: OFF, sequence complete");
                
                // После завершения можно сбросить в IDLE 
                // или оставить RUNNING для отслеживания статуса через датчик
                _motorState = MOTOR_IDLE;  // Готовы к новым командам
                _activePin = 0;
            }
            break;
            
        case MOTOR_RUNNING:
            // Мотор работает, реле отключены
            break;
            
        default:
            break;
    }
}

// STOP - как было, плюс прерывание импульсов
void ValleyController::stop() {
    Serial.println("Valley: STOP command");
    
    // Если в процессе импульсов - прерываем
    if (_motorState != MOTOR_IDLE) {
        abortPulseSequence();
        Serial.println("  Pulse sequence aborted by STOP");
    }
    
    // Импульс остановки на пин 19
    setRelay(PIN_RELAY_MAIN, true);
    delay(STOP_PULSE_DURATION_MS);  // 3 секунды блокировки
    setRelay(PIN_RELAY_MAIN, false);
    
    Serial.println("  STOP complete");
}

// Проверки состояния
bool ValleyController::isRunning() {
    return _motorState != MOTOR_IDLE;
}

bool ValleyController::isInPulseSequence() {
    return (_motorState == PULSE_1_ACTIVE || 
            _motorState == PULSE_1_PAUSE || 
            _motorState == PULSE_2_ACTIVE);
}

uint8_t ValleyController::getActiveDirection() {
    if (_activePin == PIN_RELAY_DIR_CW) return DIR_CW;
    if (_activePin == PIN_RELAY_DIR_CCW) return DIR_CCW;
    return DIR_UNKNOWN;
}

void ValleyController::setRelay(int pin, bool state) {
    // Дополнительная защита: если включаем направление, проверяем другой пин
    if (state && (pin == PIN_RELAY_DIR_CW || pin == PIN_RELAY_DIR_CCW)) {
        int otherPin = (pin == PIN_RELAY_DIR_CW) ? PIN_RELAY_DIR_CCW : PIN_RELAY_DIR_CW;
        if (digitalRead(otherPin) == LOW) {  // Если другой включен (LOW = ON для active LOW)
            Serial.println("ERROR: Attempt to enable both direction pins!");
            setRelay(otherPin, false);  // Выключаем другой
        }
    }
    
    digitalWrite(pin, state ? LOW : HIGH);  // LOW = ON для active LOW
}