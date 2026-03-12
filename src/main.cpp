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

void setupWiFi();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void processCommand(const char* command);
void processCalibration(const char* payload);
void publishAllStatus();

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
    
    if (WiFi.status() != WL_CONNECTED) {
        if (now - lastWiFiCheck > WIFI_RECONNECT_INTERVAL_MS) {
            Serial.println("WiFi lost, reconnecting...");
            setupWiFi();
            lastWiFiCheck = now;
        }
        return;
    }
    
    mqtt.loop();
    
    SensorData data = sensors.update();
    tracker.update(data.motorRunning, data.direction);
    
    if (now - lastPublishTime > MQTT_PUBLISH_INTERVAL_MS) {
        publishAllStatus();
        lastPublishTime = now;
    }
    
    delay(10);
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
        // Определяем направление: если известно текущее - используем его, иначе последнее установленное
        uint8_t dirToUse;
        if (currentState.direction != DIR_UNKNOWN) {
            dirToUse = currentState.direction;
            controller.setLastDirection(dirToUse);  // Синхронизируем
        } else {
            dirToUse = controller.getLastDirection();
        }
        
        Serial.print("START with direction: ");
        Serial.println(dirToUse == DIR_CW ? "CW" : "CCW");
        
        controller.start(dirToUse);
    } 
    else if (strcmp(command, "STOP") == 0) {
        controller.stop();
    } 
    else if (strcmp(command, "CHANGE_DIRECTION") == 0) {
        controller.changeDirection();
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

void publishAllStatus() {
    SensorData data = sensors.update();
    
    mqtt.publish("online", "true", true);
    mqtt.publish("motor_status", data.motorRunning ? "running" : "stopped", true);
    
    // Отправляем направление
    const char* dirStr;
    switch(data.direction) {
        case DIR_CW: dirStr = "clockwise"; break;
        case DIR_CCW: dirStr = "counter_clockwise"; break;
        default: dirStr = "unknown"; break;
    }
    mqtt.publish("direction", dirStr, true);
    
    mqtt.publish("runtime", (int)data.runtimeSeconds, true);
    mqtt.publish("position", tracker.getCurrentAngle(), true);
    mqtt.publish("pressure", data.pressure, true);
}