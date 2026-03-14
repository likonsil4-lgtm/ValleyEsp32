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

    int dirPin = (direction == DIR_CW) ? PIN_RELAY_DIR_CW : PIN_RELAY_DIR_CCW;

    // Шаг 1: Включаем только пин 21 (старт)
    setRelay(PIN_RELAY_START, true);
    Serial.println("Pin 21 ON");

    // Ждем 3 секунды
    delay(3000);

    // Шаг 2: Включаем пин направления (22 или 23) через 3 сек после пина 21
    setRelay(dirPin, true);
    Serial.println("Direction pin ON (3s after pin 21)");

    // Ждем еще 4 секунды (всего пин 21 работает 7 секунд)
    delay(4000);

    // Шаг 3: Выключаем пин направления (работал 4 секунды)
    setRelay(dirPin, false);
    Serial.println("Direction pin OFF");

    // Шаг 4: Выключаем пин 21 (работал 7 секунд всего)
    setRelay(PIN_RELAY_START, false);
    Serial.println("Pin 21 OFF");

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