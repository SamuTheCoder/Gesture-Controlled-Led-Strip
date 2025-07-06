#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration

#define WIFI_SSID "PRIVATE"
#define WIFI_PASS "PRIVATE"

// MQTT Configuration
#define MQTT_BROKER_URI "mqtt://192.168.1.69:1883"  // IP of Home Assistant
#define MQTT_USERNAME "esp32"
#define MQTT_PASSWORD "esp32"

// MQTT Topics
#define MQTT_TOPIC_PROXIMITY "esp32/proximity"
#define MQTT_TOPIC_GESTURE "esp32/gesture"
#define MQTT_TOPIC_STATUS "esp32/status"

#endif // CONFIG_H