#ifndef VALLEY_CONTROLLER_H
#define VALLEY_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

class ValleyController {
public:
    ValleyController();
    void begin();
    
    // Старт: одновременно включаем 21 и направление (22 или 23)
    void start(uint8_t direction);  // DIR_CW или DIR_CCW
    
    void stop();
    void changeDirection();  // Меняет направление для следующего старта
    
    uint8_t getLastDirection();
    void setLastDirection(uint8_t dir);
    
private:
    bool _mainRelayState;
    uint8_t _lastDirection;  // DIR_CW или DIR_CCW (не UNKNOWN!)
    
    void initRelays();
    void setRelay(int pin, bool state);
};

#endif