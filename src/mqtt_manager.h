#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

class MQTTManager {
public:
    MQTTManager();
    void begin();
    bool connect();
    bool isConnected();
    void loop();
    void publish(const char* subtopic, const char* payload, bool retained = false);
    void publish(const char* subtopic, float value, bool retained = false);
    void publish(const char* subtopic, int value, bool retained = false);
    void setCallback(void (*callback)(char*, byte*, unsigned int));
    
private:
    WiFiClientSecure _wifiClient;
    PubSubClient _client;
    unsigned long _lastReconnectAttempt;
    
    bool reconnect();
};

#endif