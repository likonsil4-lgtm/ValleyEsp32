#include "valley_controller.h"

ValleyController::ValleyController() {
    _mainRelayState = false;
    _lastDirection = DIR_CW;
    _targetDirection = DIR_CW;
    _motorState = MOTOR_IDLE;
    _stateStartTime = 0;
}

void ValleyController::begin() {
    initRelays();
}

void ValleyController::initRelays() {
    pinMode(PIN_RELAY_MAIN, OUTPUT);
    pinMode(PIN_RELAY_START, OUTPUT);
    pinMode(PIN_RELAY_DIR_CW, OUTPUT);
    pinMode(PIN_RELAY_DIR_CCW, OUTPUT);

    // Все выключены (HIGH для active LOW)
    digitalWrite(PIN_RELAY_MAIN, HIGH);
    digitalWrite(PIN_RELAY_START, HIGH);
    digitalWrite(PIN_RELAY_DIR_CW, HIGH);
    digitalWrite(PIN_RELAY_DIR_CCW, HIGH);

    _mainRelayState = false;
    _motorState = MOTOR_IDLE;
}

// НЕБЛОКИРУЮЩИЙ старт - управление возвращается сразу!
void ValleyController::start(uint8_t direction) {
    if (_motorState != MOTOR_IDLE) {
        Serial.println("Valley: START rejected - already in sequence");
        return;
    }
    
    Serial.print("Valley: START sequence BEGIN, direction ");
    Serial.println(direction == DIR_CW ? "CW" : "CCW");
    
    _targetDirection = direction;
    _motorState = MOTOR_STARTING_STEP1;
    _stateStartTime = millis();
    
    // Шаг 1: Включаем только пин 21
    setRelay(PIN_RELAY_START, true);
    Serial.println("  [Step 1] Pin 21 ON");
}

// НОВОЕ: вызывать в loop() каждую итерацию!
void ValleyController::update() {
    if (_motorState == MOTOR_IDLE) return;
    
    unsigned long now = millis();
    unsigned long elapsed = now - _stateStartTime;
    
    switch (_motorState) {
        case MOTOR_STARTING_STEP1:
            // Ждём 3 секунды, затем включаем направление
            if (elapsed >= 3000) {
                int dirPin = (_targetDirection == DIR_CW) ? 
                    PIN_RELAY_DIR_CW : PIN_RELAY_DIR_CCW;
                setRelay(dirPin, true);
                _motorState = MOTOR_STARTING_STEP2;
                _stateStartTime = now;  // Сбрасываем для следующего шага
                Serial.println("  [Step 2] Direction pin ON");
            }
            break;
            
        case MOTOR_STARTING_STEP2:
            // Ждём 4 секунды (направление работает), затем выключаем всё
            if (elapsed >= 4000) {
                int dirPin = (_targetDirection == DIR_CW) ? 
                    PIN_RELAY_DIR_CW : PIN_RELAY_DIR_CCW;
                setRelay(dirPin, false);
                setRelay(PIN_RELAY_START, false);
                _lastDirection = _targetDirection;
                _motorState = MOTOR_RUNNING;  // Или MOTOR_IDLE если не отслеживаем
                Serial.println("  [Step 3] Sequence complete, motor running");
            }
            break;
            
        case MOTOR_RUNNING:
            // Мотор работает нормально, ничего не делаем
            break;
            
        default:
            break;
    }
}

bool ValleyController::isInStartingSequence() {
    return _motorState == MOTOR_STARTING_STEP1 || 
           _motorState == MOTOR_STARTING_STEP2;
}

void ValleyController::stop() {
    Serial.println("Valley: STOP sequence");
    
    // Если в процессе старта - прерываем
    if (_motorState != MOTOR_IDLE) {
        // Выключаем все реле
        digitalWrite(PIN_RELAY_START, HIGH);
        digitalWrite(PIN_RELAY_DIR_CW, HIGH);
        digitalWrite(PIN_RELAY_DIR_CCW, HIGH);
        _motorState = MOTOR_IDLE;
        Serial.println("  Startup sequence aborted");
    }
    
    // Включаем пин 19 на 3 секунды
    setRelay(PIN_RELAY_MAIN, true);
    delay(STOP_PULSE_DURATION_MS);  // Этот delay приемлем - стоп редкий
    setRelay(PIN_RELAY_MAIN, false);
    
    Serial.println("  STOP complete");
}

void ValleyController::changeDirection() {
    Serial.println("Valley: CHANGE DIRECTION");
    _lastDirection = (_lastDirection == DIR_CW) ? DIR_CCW : DIR_CW;
    Serial.print("  New direction: ");
    Serial.println(_lastDirection == DIR_CW ? "CW" : "CCW");
}

uint8_t ValleyController::getLastDirection() {
    return _lastDirection;
}

void ValleyController::setLastDirection(uint8_t dir) {
    if (dir == DIR_CW || dir == DIR_CCW) {
        _lastDirection = dir;
    }
}

void ValleyController::setRelay(int pin, bool state) {
    digitalWrite(pin, state ? LOW : HIGH);
    if (pin == PIN_RELAY_MAIN) {
        _mainRelayState = state;
    }
}