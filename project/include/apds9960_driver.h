/**
 * @file apds9960_driver.h
 * @brief Driver for the APDS-9960 sensor, providing functions to initialize and interact with the sensor over I2C (only gesture and proximity features).
 */

#ifndef APDS9960_DRIVER_H
#define APDS9960_DRIVER_H

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

// APDS-9960 I2C address
#define APDS9960_I2C_ADDR 0x39

// APDS-9960 register definitions
#define APDS9960_ENABLE        0x80
#define APDS9960_GCONF1        0xA2 // Gesture Configuration One
#define APDS9960_GCONF2        0xA3 // Gesture Configuration Two
#define APDS9960_GCONF4        0xAB
#define APDS9960_GSTATUS       0xAF
#define APDS9960_GFLVL         0xAE // Gesture FIFO Level
#define APDS9960_GFIFO_U       0xFC // Gesture FIFO UP Value
#define APDS9960_ID            0x92
#define APDS9960_ID_EXPECTED   0x9E // Common valid IDs are 0xAB (Rev B), 0x9E (Rev A), some datasheets mention 0xA8.
#define APDS9960_GPENTH        0xA0 // Gesture Proximity Enter Threshold
#define APDS9960_GEXTH         0xA1 // Gesture Exit Threshold
#define APDS9960_GPULSE        0xA6 // Gesture Pulse Count and Length
#define APDS9960_PDATA         0x9C // Proximity Data
#define APDS9960_PPULSE        0x8E // Proximity Pulse Count and Length
#define APDS9960_CONTROL       0x8F // Control Register (for PGAIN, LDRIVE)

// I2C configuration
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SCL_IO 4
#define I2C_MASTER_SDA_IO 5
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_TIMEOUT_MS 1000


/**
 * @brief Initialize I2C communication with APDS-9960
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_FAIL: I2C initialization error
 */
esp_err_t apds9960_i2c_init(void);

/**
 * @brief Initialize APDS-9960 sensor with default gesture configuration
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_STATE: Sensor not responding
 */
esp_err_t apds9960_init(void);

/**
 * @brief Write a byte to APDS-9960 register
 * @param reg Register address (0x00-0xFF)
 * @param value Byte to write
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_ARG: Invalid register
 *         - ESP_FAIL: I2C communication error
 */
esp_err_t apds9960_write_byte(uint8_t reg, uint8_t value);

/**
 * @brief Read a byte from APDS-9960 register
 * @param reg Register address (0x00-0xFF)
 * @param data Pointer to store read value
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_ARG: Invalid register or NULL pointer
 *         - ESP_FAIL: I2C communication error
 */
esp_err_t apds9960_read_byte(uint8_t reg, uint8_t *data);

/**
 * @brief Read a block of data from consecutive registers
 * @param reg Starting register address
 * @param data Buffer to store read data
 * @param len Number of bytes to read (max 32)
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_SIZE: Requested length too large
 *         - ESP_FAIL: I2C communication error
 */
esp_err_t apds9960_read_block(uint8_t reg, uint8_t *data, uint8_t len);

/**
 * @brief Read proximity value from PDATA register
 * @param proximity Pointer to store proximity value (0-255)
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_STATE: Proximity engine not enabled
 */
esp_err_t apds9960_read_proximity(uint8_t *proximity);

/**
 * @brief Read gesture status from GSTATUS register
 * @param status Pointer to store status byte (bit 0 = GVALID)
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_FAIL: I2C error
 */
esp_err_t apds9960_read_gesture_status(uint8_t *status);

/**
 * @brief Read number of available gesture datasets in FIFO
 * @param level Pointer to store FIFO level (0-32)
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_FAIL: I2C error
 */
esp_err_t apds9960_read_gesture_fifo_level(uint8_t *level);

/**
 * @brief Read gesture FIFO data (4 bytes per dataset: U, D, L, R)
 * @param fifo_data Buffer to store data (must have space for 4*N bytes)
 * @return esp_err_t 
 *         - ESP_OK: Success
 *         - ESP_ERR_INVALID_SIZE: FIFO empty
 */
esp_err_t apds9960_read_gesture_fifo_data(uint8_t *fifo_data);


#endif // APDS9960_DRIVER_H