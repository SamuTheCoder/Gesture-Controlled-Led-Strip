idf_component_register(SRCS "comms.c" "gesture_led_strip.c" "project_main.c" "apds9960_driver.c" 
                       INCLUDE_DIRS "."
                        PRIV_REQUIRES esp_wifi esp_event nvs_flash mqtt driver
                        REQUIRES led_strip
                       )
