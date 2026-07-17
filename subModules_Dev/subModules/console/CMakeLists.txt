idf_component_register(
    SRCS
        "console_manager.c"

    INCLUDE_DIRS
        "."

    REQUIRES
        driver
        esp_driver_uart
)
