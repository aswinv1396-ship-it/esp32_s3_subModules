#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t audio_page_handler(httpd_req_t *req);

#endif
