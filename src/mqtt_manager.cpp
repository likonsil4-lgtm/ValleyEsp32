#include "mqtt_manager.h"

MQTTManager::MQTTManager() : _client(_wifiClient) {
    _lastReconnectAttempt = 0;
}

void MQTTManager::begin() {
    _wifiClient.setInsecure();  // Для HiveMQ Cloud SSL
    _client.setServer(MQTT_SERVER, MQTT_PORT);
}

bool MQTTManager::connect() {
    return reconnect();
}

bool MQTTManager::isConnected() {
    return _client.connected();
}

void MQTTManager::loop() {
    if (!_client.connected()) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > 5000) {
            _lastReconnectAttempt = now;
            if (reconnect()) {
                _lastReconnectAttempt = 0;
            }
        }
    } else {
        _client.loop();
    }
}

bool MQTTManager::reconnect() {
    String clientId = String(DEVICE_ID) + "_" + String(random(0xffff), HEX);
    
    Serial.print("MQTT connecting...");
    
    if (_client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
        Serial.println("connected");
        
        // Подписка на топики команд
        String cmdTopic = String("valley/") + DEVICE_ID + "/command";
        String calTopic = String("valley/") + DEVICE_ID + "/calibration";
        
        _client.subscribe(cmdTopic.c_str());
        _client.subscribe(calTopic.c_str());
        
        Serial.print("Subscribed to: ");
        Serial.println(cmdTopic);
        
        // Публикация статуса online
        publish("online", "true", true);
        return true;
    } else {
        Serial.print("failed, rc=");
        Serial.print(_client.state());
        Serial.println(" try again in 5 seconds");
        return false;
    }
}

void MQTTManager::publish(const char* subtopic, const char* payload, bool retained) {
    if (!_client.connected()) return;
    
    String topic = String("valley/") + DEVICE_ID + "/" + subtopic;
    _client.publish(topic.c_str(), payload, retained);
}

void MQTTManager::publish(const char* subtopic, float value, bool retained) {
    char buffer[16];
    dtostrf(value, 1, 1, buffer);
    publish(subtopic, buffer, retained);
}

void MQTTManager::publish(const char* subtopic, int value, bool retained) {
    char buffer[16];
    itoa(value, buffer, 10);
    publish(subtopic, buffer, retained);
}

void MQTTManager::setCallback(void (*callback)(char*, byte*, unsigned int)) {
    _client.setCallback(callback);
}