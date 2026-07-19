#ifndef SERVICE_HANDLER_H
#define SERVICE_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t service_page_handler(httpd_req_t *req);

#endif