/**
 * @file gesture_led_strip.h
 * @brief Module for controlling LED strip effects based on apds9960 gestures.
 */
#pragma once

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"

// LED strip config
#define LED_STRIP_GPIO 8

/**
 * @struct rgb_t
 * @brief Structure to hold RGB color values.
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_t;


extern rgb_t led_colors[];

extern uint8_t s_led_state;

// color index
extern int8_t i;

extern led_strip_handle_t led_strip;
extern TaskHandle_t chromatic_task_handle;
extern TaskHandle_t shift_chromatic_task_handle;
extern volatile bool chromatic_active;
extern volatile bool shift_chromatic_active;

/**
 * @brief Function to change colors of the LED strip based on gesture.asm
 * @param gesture The gesture detected by the apds9960 sensor.
 */
const char* blink_led(uint8_t gesture);

/**
 * @brief Function to configure the LED strip.
 */
void configure_led(void);

/**
 * @brief Task for applying chromatic effect
 */
void chromatic_effect_task(void *arg);

/**
 * @brief Task for applying shift chromatic effect
 */
void shift_chromatic_effect_task(void *arg);
