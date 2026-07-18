idf_component_register(
    SRCS
        "websocket_client.c"

    INCLUDE_DIRS
        "."

    REQUIRES
    	espressif__esp_websocket_client
        esp_event 
        esp_http_client
        esp_netif
        esp-tls
        mbedtls
        
   EMBED_TXTFILES
        "ngrok_ca.pem"

)
