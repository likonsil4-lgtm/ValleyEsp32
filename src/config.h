#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============== DEVICE CONFIGURATION ==============
#define DEVICE_ID "valley_4"
#define DEVICE_NAME "Valley 4"

// ============== WIFI CONFIGURATION ==============
#define WIFI_SSID "E25RA"
#define WIFI_PASSWORD "Era03072025+"

// ============== MQTT CONFIGURATION ==============
#define MQTT_SERVER "03f3df8309314ebdaf9c2d78737996db.s1.eu.hivemq.cloud"
#define MQTT_PORT 8883
#define MQTT_USER "Valley"
#define MQTT_PASS "13042004qwW+"

// ============== PIN CONFIGURATION ==============
// Input pins
#define PIN_PRESSURE_SENSOR 34
#define PIN_MOTOR_STATUS 32 // PC817 для 120VAC
#define PIN_DIRECTION_CW 33 // Направление: по часовой
#define PIN_DIRECTION_CCW 25 // Направление: против часовой

// Relay pins (Активный LOW)
#define PIN_RELAY_MAIN 19 // Основное реле (изначально ВЫКЛ, включается на 3с при STOP)
#define PIN_RELAY_START 21 // Реле старта
#define PIN_RELAY_DIR_CW 22 // Реле направления по часовой
#define PIN_RELAY_DIR_CCW 23 // Реле направления против часовой

// ============== TIMING CONFIGURATION ==============
#define MQTT_PUBLISH_INTERVAL_MS 500
#define MOTOR_STATUS_TIMEOUT_MS 5000    
#define STOP_PULSE_DURATION_MS 3000     
#define WIFI_RECONNECT_INTERVAL_MS 3000 
#define PULSE_DURATION_MS 1000
#define PULSE_PAUSE_MS 2000 

// ============== DIRECTION STATE ==============
#define DIR_UNKNOWN 0
#define DIR_CW 1 // По часовой (налево)
#define DIR_CCW 2 // Против часовой (направо)
#define CMD_START_CW "STARTCW"
#define CMD_START_CCW "STARTCCW"
#define CMD_STOP "STOP"
#define CMD_PING "PING"
// ============== DEFAULT CALIBRATION ==============
#define DEFAULT_START_ANGLE 220.0
#define DEFAULT_ROTATION_TIME_MINUTES 74.0

#endif