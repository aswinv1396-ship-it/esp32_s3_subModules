#ifndef SYSTEM_HANDLER_H
#define SYSTEM_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t system_home_handler(httpd_req_t *req);

#endif