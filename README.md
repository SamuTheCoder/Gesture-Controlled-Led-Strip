# ASE_Project

This project aimed for the development of a LED strip controlled by a APDS-9960 sensor, which is capable of detecting gestures and sending data via MQTT. The sensors were connected to an ESP32-C3 and the communication was done through I2C for the sensor and SPI for the LED strip.

## Features

- **Switch colors (left/right)**
- **Chromatic modes (up/down)**

## Modules

- **apds9960_driver**: Driver for the APDS-9960 sensor.
- **gesture_led_strip**: Implements the color switching and chromatics logic
- **comms**: Handles MQTT communication of metrics



