#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"
#include "mqtt_manager.h"
#include "sensor_manager.h"
#include "valley_controller.h"
#include "position_tracker.h"

MQTTManager mqtt;
SensorManager sensors;
ValleyController controller;
PositionTracker tracker;

unsigned long lastPublishTime = 0;
unsigned long lastWiFiCheck = 0;

// Для отслеживания изменений (push вместо poll)
bool lastMotorRunning = false;
uint8_t lastDirection = DIR_UNKNOWN;
float lastPressure = 0.0;
float lastAngle = 0.0;

void setupWiFi();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void processCommand(const char* command);
void processCalibration(const char* payload);
void publishAllStatus();
void publishIfChanged();  // НОВОЕ!

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.print("Valley Controller - ");
    Serial.println(DEVICE_ID);
    Serial.println("========================================\n");
    
    controller.begin();
    sensors.begin();
    tracker.begin();
    
    setupWiFi();
    mqtt.begin();
    mqtt.setCallback(mqttCallback);
    
    Serial.println("Setup complete. Running...");
}

void loop() {
    unsigned long now = millis();
    
    // WiFi reconnect
    if (WiFi.status() != WL_CONNECTED) {
        if (now - lastWiFiCheck > WIFI_RECONNECT_INTERVAL_MS) {
            Serial.println("WiFi lost, reconnecting...");
            setupWiFi();
            lastWiFiCheck = now;
        }
        return;
    }
    
    mqtt.loop();
    
    // НОВОЕ: обновление неблокирующего контроллера
    controller.update();
    
    // Обновление сенсоров и трекера
    SensorData data = sensors.update();
    
    // Обновляем трекер только если мотор реально работает
    // (не в процессе стартовой последовательности!)
    bool realMotorRunning = data.motorRunning && !controller.isInStartingSequence();
    tracker.update(realMotorRunning, data.direction);
    
    // НОВОЕ: мгновенная публикация при изменениях!
    publishIfChanged();
    
    // Периодическая публикация (heartbeat) - каждые 500мс
    if (now - lastPublishTime > MQTT_PUBLISH_INTERVAL_MS) {
        publishAllStatus();
        lastPublishTime = now;
    }
    
    delay(10);  // Небольшая задержка для стабильности WiFi
}

void setupWiFi() {
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nWiFi failed! Restarting...");
        ESP.restart();
    }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    Serial.print("MQTT [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);
    
    String topicStr = String(topic);
    
    if (topicStr.indexOf("/command") > 0) {
        processCommand(message);
    } else if (topicStr.indexOf("/calibration") > 0) {
        processCalibration(message);
    }
}

void processCommand(const char* command) {
    SensorData currentState = sensors.update();
    
    if (strcmp(command, "START") == 0) {
        uint8_t dirToUse;
        if (currentState.direction != DIR_UNKNOWN) {
            dirToUse = currentState.direction;
            controller.setLastDirection(dirToUse);
        } else {
            dirToUse = controller.getLastDirection();
        }
        
        Serial.print("Command START with direction: ");
        Serial.println(dirToUse == DIR_CW ? "CW" : "CCW");
        
        controller.start(dirToUse);
    } 
    else if (strcmp(command, "STOP") == 0) {
        controller.stop();
    } 
    else if (strcmp(command, "CHANGE_DIRECTION") == 0) {
        controller.changeDirection();
    }
    else if (strcmp(command, "PING") == 0) {
        // Ответ на пинг - сразу публикуем статус
        publishAllStatus();
    }
}

void processCalibration(const char* payload) {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        Serial.print("JSON error: ");
        Serial.println(error.c_str());
        return;
    }
    
    float startAngle = doc["startAngle"] | DEFAULT_START_ANGLE;
    float rotationTime = doc["rotationTimeMinutes"] | DEFAULT_ROTATION_TIME_MINUTES;
    
    tracker.setCalibration(startAngle, rotationTime);
    
    Serial.print("Calibration: angle=");
    Serial.print(startAngle);
    Serial.print(", time=");
    Serial.println(rotationTime);
}

// НОВОЕ: публикация только при изменениях - мгновенная реакция!
void publishIfChanged() {
    SensorData data = sensors.update();
    float currentAngle = tracker.getCurrentAngle();
    bool changed = false;
    
    // Мотор вкл/выкл
    if (data.motorRunning != lastMotorRunning) {
        mqtt.publish("motor_status", data.motorRunning ? "running" : "stopped", true);
        mqtt.publish("online", "true", true);  // Подтверждаем online
        lastMotorRunning = data.motorRunning;
        changed = true;
        Serial.println("PUBLISH: motor status changed");
    }
    
    // Направление (только если известно)
    if (data.direction != lastDirection && data.direction != DIR_UNKNOWN) {
        const char* dirStr = (data.direction == DIR_CW) ? "clockwise" : "counter_clockwise";
        mqtt.publish("direction", dirStr, true);
        lastDirection = data.direction;
        changed = true;
        Serial.println("PUBLISH: direction changed");
    }
    
    // Давление (с порогом 2 PSI)
    if (abs(data.pressure - lastPressure) > 2.0) {
        mqtt.publish("pressure", data.pressure, true);
        lastPressure = data.pressure;
        changed = true;
    }
    
    // Позиция (если мотор работает или изменилась значительно)
    if (data.motorRunning || abs(currentAngle - lastAngle) > 1.0) {
        mqtt.publish("position", currentAngle, true);
        lastAngle = currentAngle;
        changed = true;
    }
    
    // Время работы (только если мотор работает)
    if (data.motorRunning && data.runtimeSeconds > 0) {
        static unsigned long lastRuntimePublish = 0;
        if (millis() - lastRuntimePublish > 1000) {  // Раз в секунду
            mqtt.publish("runtime", (int)data.runtimeSeconds, true);
            lastRuntimePublish = millis();
        }
    }
}

void publishAllStatus() {
    SensorData data = sensors.update();
    float currentAngle = tracker.getCurrentAngle();
    
    mqtt.publish("online", "true", true);
    mqtt.publish("motor_status", data.motorRunning ? "running" : "stopped", true);
    
    const char* dirStr;
    switch(data.direction) {
        case DIR_CW: dirStr = "clockwise"; break;
        case DIR_CCW: dirStr = "counter_clockwise"; break;
        default: dirStr = "unknown"; break;
    }
    mqtt.publish("direction", dirStr, true);
    
    mqtt.publish("runtime", (int)data.runtimeSeconds, true);
    mqtt.publish("position", currentAngle, true);
    mqtt.publish("pressure", data.pressure, true);
    
    // Обновляем last-переменные
    lastMotorRunning = data.motorRunning;
    lastDirection = data.direction;
    lastPressure = data.pressure;
    lastAngle = currentAngle;
}