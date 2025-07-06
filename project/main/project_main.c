#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "../include/apds9960_driver.h"
#include "../include/gesture_led_strip.h"
#include "../include/comms.h"

static const char *TAG = "GESTURE";

uint8_t gesture = 0;

void gesture_task(void *pvParam) {
    uint8_t gstatus = 0;
    uint8_t gflvl = 0;
    uint8_t fifo_data[4]; // Buffer for one gesture dataset (U, D, L, R)
    uint8_t pdata = 0; // For proximity data

    ESP_LOGI(TAG, "Gesture task started.");
    while (1) {
        // Read Proximity Data
        if (apds9960_read_byte(APDS9960_PDATA, &pdata) == ESP_OK) {
            ESP_LOGI(TAG, "Proximity: %d", pdata);
        } else {
            ESP_LOGE(TAG, "Failed to read PDATA");
        }

        if(pdata <= 140){
            ESP_LOGI(TAG, "Proximity too low, skipping gesture detection.");
            vTaskDelay(pdMS_TO_TICKS(200)); // Wait before next check
            continue;
        }
        if (apds9960_read_byte(APDS9960_GSTATUS, &gstatus) == ESP_OK) {
            ESP_LOGI(TAG, "GSTATUS: 0x%02X", gstatus); 
            char proximity_str[10];
            snprintf(proximity_str, sizeof(proximity_str), "%d", pdata);
            publish("esp32/proximity", proximity_str); // Publish proximity data
            if (gstatus & 0x01) { // Check GVALID bit
                // ESP_LOGI(TAG, "GVALID set, checking FIFO level...");
                if (apds9960_read_byte(APDS9960_GFLVL, &gflvl) == ESP_OK) {
                    if (gflvl > 0) {
                        ESP_LOGI(TAG, "Gesture data in FIFO! Level: %d", gflvl);
                        if (apds9960_read_block(APDS9960_GFIFO_U, fifo_data, 4) == ESP_OK) {
                            ESP_LOGI(TAG, "Gesture Data: U=%3d, D=%3d, L=%3d, R=%3d",
                                     fifo_data[0], fifo_data[1], fifo_data[2], fifo_data[3]);
                            if (fifo_data[0] > 50 && fifo_data[0] > fifo_data[1] && fifo_data[0] > fifo_data[2] && fifo_data[0] > fifo_data[3]) {
                                ESP_LOGW(TAG, "Tentative GESTURE: UP");
                                publish("esp32/gesture", "UP");                        
                                gesture = 1;
                            } else if (fifo_data[1] > 50 && fifo_data[1] > fifo_data[0] && fifo_data[1] > fifo_data[2] && fifo_data[1] > fifo_data[3]) {
                                ESP_LOGW(TAG, "Tentative GESTURE: DOWN");
                                publish("esp32/gesture", "DOWN");
                                gesture = 2;
                            } else if (fifo_data[2] > 50 && fifo_data[2] > fifo_data[0] && fifo_data[2] > fifo_data[1] && fifo_data[2] > fifo_data[3]) {
                                ESP_LOGW(TAG, "Tentative GESTURE: LEFT");
                                publish("esp32/gesture", "LEFT");
                                gesture = 3;
                            } else if (fifo_data[3] > 50 && fifo_data[3] > fifo_data[0] && fifo_data[3] > fifo_data[1] && fifo_data[3] > fifo_data[2]) {
                                ESP_LOGW(TAG, "Tentative GESTURE: RIGHT");
                                publish("esp32/gesture", "RIGHT");
                                gesture = 4;
                            }
                            blink_led(gesture);
                        } else {
                            ESP_LOGE(TAG, "Failed to read gesture FIFO data.");
                        }
                    }
                } else {
                    ESP_LOGE(TAG, "Failed to read GFLVL.");
                }
            } else { 
            }
        } else {
            ESP_LOGE(TAG, "Failed to read GSTATUS.");
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Poll rate
    }
}

void app_main(void) {
    // Initialize GPIO for LED
    gpio_reset_pin(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);

    // Initialize NVS
    nvs_flash_init();
    
    // Initialize WiFi
    wifi_init();
    
    // Initialize I2C and APDS-9960
    apds9960_i2c_init();
    apds9960_init();

    configure_led();
    vTaskDelay(pdMS_TO_TICKS(5000)); // 5-second delay

    // Initialize MQTT
    mqtt_init();

    xTaskCreate(gesture_task, "gesture_task", 4096, NULL, 3, NULL);  
    
    ESP_LOGI(TAG, "Both proximity and gesture tasks started");
}