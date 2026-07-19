idf_component_register(

    SRCS
        "webportal.c"
        "routes.c"

        "handlers/system_handler.c"
        "handlers/network_handler.c"
        "handlers/service_handler.c"

    INCLUDE_DIRS
        "."
        "handlers"

    REQUIRES
        esp_http_server

)