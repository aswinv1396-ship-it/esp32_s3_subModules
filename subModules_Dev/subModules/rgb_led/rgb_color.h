#ifndef RGB_COLOR_H
#define RGB_COLOR_H

#include <stdint.h>
#include "esp_err.h"


typedef enum
{
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_WHITE,
    COLOR_YELLOW,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_ORANGE,
    COLOR_PURPLE,
    COLOR_PINK,
    COLOR_LIME,
    COLOR_TEAL,
    COLOR_GOLD,
    COLOR_VIOLET,
    COLOR_SKY_BLUE,
    COLOR_INDIGO,
    COLOR_CORAL,
    COLOR_SALMON,
    COLOR_LAVENDER,
    COLOR_MINT,
    COLOR_OLIVE,
    COLOR_GRAY,
    COLOR_WARM_WHITE

} rgb_color_t;



/**
 * Set RGB LED using color name
 *
 * Example:
 * rgb_color_set("red",80);
 */
esp_err_t rgb_color_set(const char *color_name,
                        uint8_t brightness);



/**
 * Set RGB LED using enum
 *
 * Example:
 * rgb_color_set_enum(COLOR_BLUE,100);
 */
esp_err_t rgb_color_set_enum(rgb_color_t color,
                             uint8_t brightness);



#endif
