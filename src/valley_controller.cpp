#include "valley_controller.h"

ValleyController::ValleyController() {
    _mainRelayState = false; // Изначально ВЫКЛ
    _lastDirection = DIR_CW; // По умолчанию по часовой
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
    // Пин 19 теперь тоже выключен изначально!
    digitalWrite(PIN_RELAY_MAIN, HIGH);  // ВЫКЛ (было LOW - ВКЛ)
    digitalWrite(PIN_RELAY_START, HIGH); // ВЫКЛ
    digitalWrite(PIN_RELAY_DIR_CW, HIGH); // ВЫКЛ
    digitalWrite(PIN_RELAY_DIR_CCW, HIGH); // ВЫКЛ

    _mainRelayState = false;
}

void ValleyController::start(uint8_t direction) {
    Serial.print("Valley: START with direction ");
    Serial.println(direction == DIR_CW ? "CW" : "CCW");

    // ОДНОВРЕМЕННО включаем реле старта и направления!
    int dirPin = (direction == DIR_CW) ? PIN_RELAY_DIR_CW : PIN_RELAY_DIR_CCW;

    // Включаем оба реле одновременно
    setRelay(PIN_RELAY_START, true);
    setRelay(dirPin, true);

    Serial.println("Relays 21 and direction relay ON simultaneously");

    // Ждем 3 секунды
    delay(START_PULSE_DURATION_MS);

    // Выключаем оба одновременно
    setRelay(PIN_RELAY_START, false);
    setRelay(dirPin, false);

    Serial.println("Relays OFF");

    // Сохраняем направление
    _lastDirection = direction;
}

void ValleyController::stop() {
    Serial.println("Valley: STOP sequence - Activating pin 19 for 3 seconds");

    // Включаем пин 19 на 3 секунды (НОВАЯ ЛОГИКА!)
    setRelay(PIN_RELAY_MAIN, true);  // ВКЛ (LOW для active LOW)
    delay(STOP_PULSE_DURATION_MS);
    setRelay(PIN_RELAY_MAIN, false); // ВЫКЛ (HIGH для active LOW)

    Serial.println("Pin 19 deactivated");
}

void ValleyController::changeDirection() {
    Serial.println("Valley: CHANGE DIRECTION requested");
    // Меняем направление для следующего старта
    _lastDirection = (_lastDirection == DIR_CW) ? DIR_CCW : DIR_CW;
    Serial.print("New direction set to: ");
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
    // Active LOW: LOW = ON, HIGH = OFF
    digitalWrite(pin, state ? LOW : HIGH);

    if (pin == PIN_RELAY_MAIN) {
        _mainRelayState = state;
    }
}