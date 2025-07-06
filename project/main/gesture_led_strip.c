#include "../include/gesture_led_strip.h"
#include "../include/comms.h"

static const char *TAG_LED = "LED_STRIP";
TaskHandle_t chromatic_task_handle = NULL;
TaskHandle_t shift_chromatic_task_handle = NULL;
volatile bool chromatic_active = false;
volatile bool shift_chromatic_active = false;


rgb_t led_colors[] = {
    {74, 0, 105},    // Violet (50% brightness)
    {69, 21, 113},   // Blue Violet (50% brightness)
    {37, 0, 65},     // Indigo (50% brightness)
    {0, 0, 128},     // Blue (50% brightness)
    {0, 128, 128},   // Cyan (50% brightness)
    {0, 128, 0},     // Green (50% brightness)
    {128, 128, 0},   // Yellow (50% brightness)
    {128, 83, 0},    // Orange (50% brightness)
    {128, 34, 0},    // Red Orange (50% brightness)
    {128, 0, 0}      // Red (50% brightness)
};

// Color names array that matches the colors
const char* color_names[] = {
    "Violet",
    "Blue Violet",
    "Indigo",
    "Blue",
    "Cyan",
    "Green",
    "Yellow",
    "Orange",
    "Red Orange",
    "Red"
};

uint8_t s_led_state = 0;
int8_t i = 0; // Index for the current color in led_colors
led_strip_handle_t led_strip;

const char* blink_led(uint8_t gesture)
{
    // Stop chromatic if running and a new gesture is received
    if (gesture != 1 && chromatic_active) {
        chromatic_active = false;
        vTaskDelay(pdMS_TO_TICKS(50)); // Give the task time to end gracefully
    }
    // Stop shift chromatic if running and a new gesture is received
    if (gesture != 2 && shift_chromatic_active) {
        shift_chromatic_active = false;
        vTaskDelay(pdMS_TO_TICKS(50)); // Give the task time to end gracefully
    }

    /* If the addressable LED is enabled */
    switch (gesture)
    {
    case 1:
        /* If gesture is up, make a chromatic change */
        if (!chromatic_active) {
            chromatic_active = true;
            xTaskCreate(chromatic_effect_task, "chromatic", 2048, NULL, 5, &chromatic_task_handle);
        }
        break;
        

        break;
    case 2:
        /* If gesture is down, shift chromatic effect */
        if (!shift_chromatic_active) {
            shift_chromatic_active = true;
            xTaskCreate(shift_chromatic_effect_task, "shift_chromatic", 2048, NULL, 5, NULL);
        }
        break;
    case 3:
        /* If the gesture is left, decrease the index */
        i--;
        if (i < 0) i = sizeof(led_colors) / sizeof(led_colors[0]) - 1; // Wrap around
        ESP_LOGI(TAG_LED, "Gesture LEFT detected, changing color to index %d", i);
        for(int j = 0; j < 29; j++) {
            led_strip_set_pixel(led_strip, j, led_colors[i].r, led_colors[i].g, led_colors[i].b);
        }

        //* Publish the new color name to MQTT */
        publish("esp32/color", color_names[i]);

        break;
    case 4:
        /* If the gesture is right, increase the index */
        i++;
        if (i >= sizeof(led_colors) / sizeof(led_colors[0])) i = 0; // Wrap around
        ESP_LOGI(TAG_LED, "Gesture RIGHT detected, changing color to index %d", i);
        for(int j = 0; j < 29; j++) {
            led_strip_set_pixel(led_strip, j, led_colors[i].r, led_colors[i].g, led_colors[i].b);
        }

        //* Publish the new color name to MQTT */
        publish("esp32/color", color_names[i]);
        break;
    default:
        break;
    }

    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
    return color_names[i];
}

void configure_led(void)
{
    ESP_LOGI(TAG_LED, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_STRIP_GPIO,
        .max_leds = 30, // at least one LED on board
    };

    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_set_pixel(led_strip, 0, 0, 0, 0);
}

void chromatic_effect_task(void *arg) {
    int color_index = 0;

    // Publish MQTT
    publish("esp32/color", "Chromatic Effect");

    while (chromatic_active) {
        rgb_t color = led_colors[color_index];
        for (int j = 0; j < 29; j++) {
            led_strip_set_pixel(led_strip, j, color.r, color.g, color.b);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(200)); // 200ms delay

        color_index = (color_index + 1) % (sizeof(led_colors)/sizeof(led_colors[0]));
    }

    vTaskDelete(NULL); // End the task when no longer active
}

void shift_chromatic_effect_task(void *arg) {
    int shift_index = 0;
    int num_colors = sizeof(led_colors) / sizeof(led_colors[0]);

    // Publish MQTT
    publish("esp32/color", "Shift Chromatic Effect");

    while (shift_chromatic_active) {
        for (int j = 0; j < 29; j++) {
            rgb_t color = led_colors[(j + shift_index) % num_colors];
            led_strip_set_pixel(led_strip, j, color.r, color.g, color.b);
        }
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(150)); // tweak speed here

        shift_index = (shift_index + 1) % num_colors;
    }

    led_strip_refresh(led_strip);
    vTaskDelete(NULL);
}