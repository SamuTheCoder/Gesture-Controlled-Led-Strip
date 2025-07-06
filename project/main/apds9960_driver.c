#include "../include/apds9960_driver.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "APDS9960";

esp_err_t apds9960_i2c_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
    ESP_LOGI(TAG, "I2C Master initialized.");
    return ESP_OK;
}

esp_err_t apds9960_write_byte(uint8_t reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Write to reg 0x%02X failed: %s", reg, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t apds9960_read_byte(uint8_t reg, uint8_t *data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read from reg 0x%02X failed: %s", reg, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t apds9960_read_block(uint8_t reg, uint8_t *data, uint8_t len) {
    if (len == 0) return ESP_OK;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true); // Send the register to read from
    i2c_master_start(cmd); // Repeated start
    i2c_master_write_byte(cmd, (APDS9960_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK); // Last byte with NACK
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read block from reg 0x%02X failed: %s", reg, esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t apds9960_init(void) {
    uint8_t id = 0;
    if (apds9960_read_byte(APDS9960_ID, &id) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read sensor ID during init.");
        return ESP_FAIL;
    }

    if (id != APDS9960_ID_EXPECTED && id != 0xAB && id != 0xA8) {
        ESP_LOGE(TAG, "Wrong ID: 0x%02X. Expected 0x9E, 0xAB, or 0xA8.", id);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Sensor ID verified: 0x%02X", id);

    ESP_LOGI(TAG, "Configuring APDS-9960...");
    apds9960_write_byte(APDS9960_ENABLE, 0x00); // Disable everything first
    vTaskDelay(pdMS_TO_TICKS(10));

    // Proximity Configuration
    ESP_LOGI(TAG, "Setting Proximity: PPULSE to 0x5F (32 pulses, 16us), CONTROL to 0x0C (PGAIN 8x)");
    apds9960_write_byte(APDS9960_PPULSE, 0x5F);
    apds9960_write_byte(APDS9960_CONTROL, 0x0C);

    // Gesture Configuration
    apds9960_write_byte(APDS9960_GPENTH, 40);
    apds9960_write_byte(APDS9960_GEXTH, 30);
    apds9960_write_byte(APDS9960_GCONF1, 0x40);
    apds9960_write_byte(APDS9960_GCONF2, 0x20);
    apds9960_write_byte(APDS9960_GPULSE, 0xC9);

    // Enable Power (PON), Proximity (PEN), and Gesture (GEN)
    apds9960_write_byte(APDS9960_ENABLE, 0x45); // PON | PEN | GEN = 0b01000101
    vTaskDelay(pdMS_TO_TICKS(10));

    // Enable Gesture Mode (GMODE=1) in GCONF4
    apds9960_write_byte(APDS9960_GCONF4, 0x01); // GMODE=1
    ESP_LOGI(TAG, "APDS-9960 configured and gesture mode enabled.");
    
    return ESP_OK;
}

esp_err_t apds9960_read_proximity(uint8_t *proximity) {
    return apds9960_read_byte(APDS9960_PDATA, proximity);
}

esp_err_t apds9960_read_gesture_status(uint8_t *status) {
    return apds9960_read_byte(APDS9960_GSTATUS, status);
}

esp_err_t apds9960_read_gesture_fifo_level(uint8_t *level) {
    return apds9960_read_byte(APDS9960_GFLVL, level);
}

esp_err_t apds9960_read_gesture_fifo_data(uint8_t *fifo_data) {
    return apds9960_read_block(APDS9960_GFIFO_U, fifo_data, 4);
}