#include "routes.h"

#include "esp_log.h"

#include "handlers/home_handler.h"
#include "handlers/network_handler.h"
#include "handlers/audio_handler.h"
#include "handlers/services_handler.h"
#include "handlers/system_handler.h"
#include "handlers/wifi_handler.h"

static const char *TAG = "ROUTES";
/*
 * Home Page
 * URL: /
 */
static httpd_uri_t home_uri =
{
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = home_page_handler,
    .user_ctx = NULL
};

/*
 * Network Page
 * URL: /network
 */
static httpd_uri_t network_uri =
{
    .uri      = "/network",
    .method   = HTTP_GET,
    .handler  = network_page_handler,
    .user_ctx = NULL
};

/*
 * Audio Page
 * URL: /audio
 */
static httpd_uri_t audio_uri =
{
    .uri      = "/audio",
    .method   = HTTP_GET,
    .handler  = audio_page_handler,
    .user_ctx = NULL
};

/*
 * Services Page
 * URL: /services
 */
static httpd_uri_t services_uri =
{
    .uri      = "/services",
    .method   = HTTP_GET,
    .handler  = services_page_handler,
    .user_ctx = NULL
};

/*
 * System Page
 * URL: /system
 */
static httpd_uri_t system_uri =
{
    .uri      = "/system",
    .method   = HTTP_GET,
    .handler  = system_page_handler,
    .user_ctx = NULL
};

static esp_err_t favicon_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "204 No Content");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}


static httpd_uri_t favicon_uri =
{
    .uri = "/favicon.ico",
    .method = HTTP_GET,
    .handler = favicon_handler,
    .user_ctx = NULL
};

static const httpd_uri_t wifi_scan_uri =
{
    .uri = "/api/wifi/scan",
    .method = HTTP_GET,
    .handler = wifi_scan_handler,
    .user_ctx = NULL
};

static const httpd_uri_t wifi_select_uri =
{
    .uri = "/api/wifi/select",
    .method = HTTP_GET,
    .handler = wifi_select_handler,
    .user_ctx = NULL
};

static const httpd_uri_t wifi_connect_uri =
{
    .uri = "/api/wifi/connect",
    .method = HTTP_POST,
    .handler = wifi_connect_handler,
    .user_ctx = NULL
};

static const httpd_uri_t wifi_status_uri =
{
    .uri = "/api/wifi/status",
    .method = HTTP_GET,
    .handler = wifi_status_handler,
    .user_ctx = NULL
};

esp_err_t routes_register(httpd_handle_t server)
{
	ESP_LOGI(TAG,"Registering web routes");
	
    httpd_register_uri_handler( server, &home_uri);
    httpd_register_uri_handler( server, &network_uri);
    httpd_register_uri_handler( server, &audio_uri);
    httpd_register_uri_handler( server, &services_uri);
    httpd_register_uri_handler( server, &system_uri);
    httpd_register_uri_handler( server, &favicon_uri);
    httpd_register_uri_handler( server, &wifi_scan_uri);
	httpd_register_uri_handler( server, &wifi_select_uri);
	httpd_register_uri_handler( server, &wifi_connect_uri);
	httpd_register_uri_handler( server, &wifi_status_uri);
	
	ESP_LOGI(TAG,"Registering web routes completed");
    return ESP_OK;
}
