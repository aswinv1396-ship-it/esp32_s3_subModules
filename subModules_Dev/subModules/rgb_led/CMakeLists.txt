idf_component_register(
    SRCS 
        "rgb_led.c"
        "rgb_color.c"

    INCLUDE_DIRS "."

    REQUIRES
        led_strip
        esp_common
)
