idf_component_register(
    SRCS
        "main.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        rgb_led
        button
        mic
        audio
        wifi
        console
        webSocket
        time
        webPortal
        oled
)
