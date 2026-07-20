#ifndef OLED_H
#define OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

// Define our targeted hardware I2C pin mappings
#define OLED_I2C_SDA_PIN    4
#define OLED_I2C_SCL_PIN    5
#define OLED_I2C_ADDRESS    0x3C  // Standard factory I2C address for 0.96 OLEDs

/**
 * @brief Initializes I2C master driver and sets up the SSD1306 OLED controller configuration.
 * @return ESP_OK on initialization success.
 */
esp_err_t oled_init(void);

/**
 * @brief Purges screen memory buffer arrays cleanly to black.
 * @return ESP_OK on success.
 */
esp_err_t oled_clear(void);

/**
 * @brief Renders a basic ASCII text string directly onto a specific line row index.
 * 
 * @param line Row line selection (0 to 3 for standard 8px text heights)
 * @param text The ASCII character string literal layout to plot.
 * @return ESP_OK on transfer success.
 */
esp_err_t oled_write_line(uint8_t line, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* OLED_H */

