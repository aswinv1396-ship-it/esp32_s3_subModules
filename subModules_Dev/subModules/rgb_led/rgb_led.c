#include "rgb_led.h"

#include <stdio.h>

#include "led_strip.h"
#include "esp_log.h"


#define RGB_LED_GPIO 48
#define LED_COUNT     1


static const char *TAG = "RGB_LED";


static led_strip_handle_t led_strip;


/**
 * Initialize WS2812 RGB LED
 */
esp_err_t rgb_led_init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED on GPIO %d", RGB_LED_GPIO);


    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = LED_COUNT,
    };


    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
    };


    esp_err_t ret = led_strip_new_rmt_device(
                        &strip_config,
                        &rmt_config,
                        &led_strip);


    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "RGB LED initialization failed");
        return ret;
    }


    led_strip_clear(led_strip);


    ESP_LOGI(TAG, "RGB LED initialized successfully");


    return ESP_OK;
}



/**
 * Set RGB color
 */
esp_err_t rgb_led_set_color(uint8_t red,
                            uint8_t green,
                            uint8_t blue,
                            uint8_t brightness)
{

    if(brightness > 100)
    {
        brightness = 100;
        ESP_LOGW(TAG,
                 "Brightness limited to 100%%");
    }


    /*
     * Convert percentage brightness
     *
     * Example:
     *
     * red = 255
     * brightness = 50
     *
     * output = 127
     */
    uint8_t r = (red   * brightness) / 100;
    uint8_t g = (green * brightness) / 100;
    uint8_t b = (blue  * brightness) / 100;


    ESP_LOGI(TAG,
             "RGB -> R:%d G:%d B:%d Brightness:%d%%",
             r,g,b,brightness);



    /*
     * Set pixel color
     */
    led_strip_set_pixel(
            led_strip,
            0,
            r,
            g,
            b);


    /*
     * Send data to LED
     */
    led_strip_refresh(led_strip);


    return ESP_OK;
}



/**
 * Turn LED OFF
 */
esp_err_t rgb_led_off(void)
{
    ESP_LOGI(TAG,"Turning RGB LED OFF");

    led_strip_clear(led_strip);

    return ESP_OK;
}
