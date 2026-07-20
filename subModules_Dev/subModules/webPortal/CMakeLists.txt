idf_component_register(

    SRCS
        "webportal.c"
        "routes.c"

		"handlers/home_handler.c"
        "handlers/system_handler.c"
        "handlers/network_handler.c"
        "handlers/services_handler.c"
        "handlers/audio_handler.c"
        "handlers/wifi_handler.c"
        
        "html/common_nav.c"
        "html/common_css.c"
        "html/home_html.c"
        "html/network_html.c"
        "html/audio_html.c"
        "html/services_html.c"
        "html/system_html.c"

    INCLUDE_DIRS
        "."
        "handlers"
        "html"
        
    PRIV_REQUIRES
        wifi

    REQUIRES
    	esp_wifi
        esp_netif
        nvs_flash
        esp_event
        esp_http_server

)
