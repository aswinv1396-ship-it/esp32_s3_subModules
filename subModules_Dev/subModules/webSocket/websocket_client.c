#include "websocket_client.h"

#include "esp_websocket_client.h"
#include "esp_log.h"
#include "time.h"
#include <string.h>
#include "esp_crt_bundle.h"

#include "esp_heap_caps.h"

static const char *TAG = "WEBSOCKET";

bool ws_connected = false;

static esp_websocket_client_handle_t client = NULL;

static void websocket_event_handler(void *handler_args, esp_event_base_t base,int32_t event_id,void *event_data)
{

    //ESP_LOGI( TAG, "WebSocket event id = %ld", event_id );
    
    switch(event_id)
    {
    case WEBSOCKET_EVENT_CONNECTED:
        ws_connected = true;
        ESP_LOGI( TAG, "CONNECTED"  );
        break;

    case WEBSOCKET_EVENT_DISCONNECTED:
        ws_connected = false;
        ESP_LOGW( TAG, "DISCONNECTED" );
        break;

    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGE(TAG,"ERROR" );
        break;

    case WEBSOCKET_EVENT_DATA:
    {
        esp_websocket_event_data_t *data =  (esp_websocket_event_data_t *)event_data;
        ESP_LOGI(TAG, "RX data len=%d", data->data_len);
        break;
    }

    default:
        ESP_LOGI(TAG, "OTHER EVENT" );
        break;
    }
}

esp_err_t websocket_client_init( const char *url)
{
    if (url == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }
	esp_log_level_set( "websocket_client", ESP_LOG_VERBOSE);
	esp_log_level_set( "esp-tls", ESP_LOG_VERBOSE);
	esp_log_level_set( "mbedtls", ESP_LOG_VERBOSE);
	esp_log_level_set( "transport_ws", ESP_LOG_VERBOSE);
    esp_log_level_set( "transport_base", ESP_LOG_VERBOSE);
	
    time_t now =  time(NULL);

    ESP_LOGI( "TIME", "WebSocket module Unix time: %lld",  (long long)now );

	esp_websocket_client_config_t websocket_cfg =
	{
	    .uri = url,
		.crt_bundle_attach = esp_crt_bundle_attach,
        .reconnect_timeout_ms = 5000,
        .network_timeout_ms = 10000,
        .user_agent = "ESP32S3-Client",
        .task_stack = 10240,  
        .buffer_size = 2048,  
	    .skip_cert_common_name_check = true,
	};

    ESP_LOGI( TAG, "Largest internal block before WebSocket init: %lu", (unsigned long) heap_caps_get_largest_free_block( MALLOC_CAP_INTERNAL));
    ESP_LOGI( TAG, "Free heap before WebSocket init: %lu",(unsigned long)esp_get_free_heap_size());
    
    client = esp_websocket_client_init( &websocket_cfg );
    if(client == NULL)
    {
        return ESP_FAIL;
    }

    esp_websocket_register_events( client, WEBSOCKET_EVENT_ANY, websocket_event_handler, NULL );
    esp_err_t ret =  esp_websocket_client_start( client);
    if(ret == ESP_OK)
    {
        ESP_LOGI( TAG,  "WebSocket started"  );
    }
    return ret;
}

bool websocket_client_is_connected(void)
{
    if (client == NULL)
    {
        return false;
    }
    return esp_websocket_client_is_connected(client);
}

esp_err_t websocket_send_text(const char *message)
{
    static int cnt = 0;
    char tx_msg[256];

    //ESP_LOGI(TAG, "websocket_send_text() called");
    if(client == NULL)
    {
        ESP_LOGE(TAG, "Client handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    //ESP_LOGI(TAG, "Client handle OK: %p", client);
    //ESP_LOGI( TAG, "ws_connected flag = %d",   ws_connected);

    bool connected = esp_websocket_client_is_connected(client);

    //ESP_LOGI(TAG, "ESP websocket connected state = %d", connected);

    if (!connected) 
    {
        ESP_LOGW(TAG, "Websocket not connected yet");
        return ESP_ERR_INVALID_STATE;
    }

    snprintf(tx_msg, sizeof(tx_msg), "%s%d", message, cnt++);

    int len = strlen(tx_msg);
    //ESP_LOGI(TAG, "Sending %d bytes: %s", len, message);

    int sent = esp_websocket_client_send_text(client, tx_msg, len, portMAX_DELAY);
    if (sent < 0) 
    {
        ESP_LOGE(TAG, "esp_websocket_client_send_text failed");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Successfully sent %d bytes", sent);
    return ESP_OK;
}

void websocket_client_stop(void)
{
    if(client)
    {
        esp_websocket_client_stop(client);
        esp_websocket_client_destroy(client);
        client = NULL;
    }
}
