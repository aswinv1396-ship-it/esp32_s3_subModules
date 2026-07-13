#include "rgb_color.h"
#include "rgb_led.h"

#include <string.h>


typedef struct
{
    const char *name;

    uint8_t r;
    uint8_t g;
    uint8_t b;

} color_entry_t;



static const color_entry_t color_table[] =
{

    {"red",        255,0,0},
    {"green",      0,255,0},
    {"blue",       0,0,255},
    {"white",      255,255,255},

    {"yellow",     255,255,0},
    {"cyan",       0,255,255},
    {"magenta",    255,0,255},

    {"orange",     255,128,0},
    {"purple",     128,0,255},
    {"pink",       255,80,150},

    {"lime",       120,255,0},
    {"teal",       0,180,180},
    {"gold",       255,170,0},

    {"violet",     150,0,255},
    {"skyblue",    80,180,255},
    {"indigo",     75,0,130},

    {"coral",      255,100,80},
    {"salmon",     255,120,100},

    {"lavender",   180,150,255},
    {"mint",       100,255,180},

    {"olive",      128,128,0},
    {"gray",       128,128,128},

    {"warmwhite",  255,180,100}

};



esp_err_t rgb_color_set(const char *color_name,
                        uint8_t brightness)
{

    for(int i=0;
        i < sizeof(color_table)/sizeof(color_table[0]);
        i++)
    {

        if(strcmp(color_name,color_table[i].name)==0)
        {

            return rgb_led_set_color(
                    color_table[i].r,
                    color_table[i].g,
                    color_table[i].b,
                    brightness);
        }

    }


    // unknown color
    return rgb_led_off();
}




esp_err_t rgb_color_set_enum(rgb_color_t color,
                             uint8_t brightness)
{

    if(color >= sizeof(color_table)/sizeof(color_table[0]))
    {
        return ESP_ERR_INVALID_ARG;
    }


    return rgb_led_set_color(
            color_table[color].r,
            color_table[color].g,
            color_table[color].b,
            brightness);
}
