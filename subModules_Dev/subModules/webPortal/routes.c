#include "routes.h"

#include "handlers/system_handler.h"
#include "handlers/network_handler.h"
#include "handlers/service_handler.h"

static httpd_uri_t home =
{
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = system_home_handler,
    .user_ctx = NULL
};

static httpd_uri_t network_uri =
{
    .uri = "/network",
    .method = HTTP_GET,
    .handler = network_page_handler,
    .user_ctx = NULL
};

static httpd_uri_t service_uri =
{
    .uri = "/services",
    .method = HTTP_GET,
    .handler = service_page_handler,
    .user_ctx = NULL
};


esp_err_t routes_register(httpd_handle_t server)
{
    httpd_register_uri_handler(server, &home);
    httpd_register_uri_handler(server, &network_uri);
    httpd_register_uri_handler(server, &service_uri);
    return ESP_OK;
}

