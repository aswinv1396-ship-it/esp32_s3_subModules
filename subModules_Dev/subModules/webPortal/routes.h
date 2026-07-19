#ifndef ROUTES_H
#define ROUTES_H

#include "esp_http_server.h"
#include "esp_err.h"

esp_err_t routes_register(httpd_handle_t server);

#endif