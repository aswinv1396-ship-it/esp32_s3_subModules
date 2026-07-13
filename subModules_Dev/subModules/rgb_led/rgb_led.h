#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Initialize RGB LED hardware
 *
 * Configures GPIO and initializes the LED driver.
 *
 * @return ESP_OK on success
 */
esp_err_t rgb_led_init(void);


/**
 * @brief Set RGB LED color with brightness control
 *
 * @param red        Red value (0-255)
 * @param green      Green value (0-255)
 * @param blue       Blue value (0-255)
 * @param brightness Overall brightness percentage (0-100)
 *
 * Example:
 * rgb_led_set_color(255,0,0,50);
 * -> Red color at 50% brightness
 */
esp_err_t rgb_led_set_color(uint8_t red,
                            uint8_t green,
                            uint8_t blue,
                            uint8_t brightness);


/**
 * @brief Turn off RGB LED
 */
esp_err_t rgb_led_off(void);


#endif // RGB_LED_H
