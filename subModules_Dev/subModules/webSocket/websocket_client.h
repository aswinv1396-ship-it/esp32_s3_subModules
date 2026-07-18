#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>

esp_err_t websocket_client_init(const char *url);

esp_err_t websocket_send_text(const char *message);

void websocket_client_stop(void);

bool websocket_client_is_connected(void);

#endif
