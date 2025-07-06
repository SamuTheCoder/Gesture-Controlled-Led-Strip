/**
 * @file comms.h
 * @brief Module for handling Wi-Fi and MQTT communication.
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "../include/env.h"

extern esp_mqtt_client_handle_t mqtt_client;
extern bool mqtt_connected;

/**
 * @brief Function to publish a message to a specific MQTT topic.
 * @param topic The MQTT topic to publish to.
 * @param message The message to publish.
 */
void publish(const char* topic, const char* message);

/**
 * @brief Function to initialize Wi-Fi.
 */
void wifi_init();


/**
 * @brief Function to initialize MQTT.
 */
void mqtt_init();

/**
 * @brief Function to handle MQTT events.
 * @param handler_args Arguments passed to the event handler.
 * @param base The event base.
 * @param event_id The event ID.
 * @param event_data Data associated with the event.
 */
void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);