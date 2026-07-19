#include "webportal.h"

#include "esp_http_server.h"
#include "esp_log.h"
#include "routes.h"

static const char *TAG = "WEB_PORTAL";

/*-----------------------------------------------------------
 * Private Variables
 *----------------------------------------------------------*/

static httpd_handle_t server = NULL;

/*-----------------------------------------------------------
 * Public Functions
 *----------------------------------------------------------*/

esp_err_t webportal_init(void)
{
    if(server != NULL)
    {
        ESP_LOGW(TAG, "WebPortal already running");
        return ESP_OK;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.server_port = 80;

    config.max_uri_handlers = 32;

    config.stack_size = 8192;

    config.lru_purge_enable = true;

    ESP_LOGI(TAG, "Starting HTTP Server...");

    esp_err_t ret = httpd_start(&server, &config);

    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG, "HTTP Server start failed");

        server = NULL;

        return ret;
    }

    ESP_LOGI(TAG, "HTTP Server started");

    routes_register(server);

    return ESP_OK;
}

/*-----------------------------------------------------------*/

esp_err_t webportal_stop(void)
{
    if(server == NULL)
    {
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Stopping HTTP Server");

    httpd_stop(server);

    server = NULL;

    return ESP_OK;
}

/*-----------------------------------------------------------*/

httpd_handle_t webportal_get_handle(void)
{
    return server;
}