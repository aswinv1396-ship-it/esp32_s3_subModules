#ifndef WEBSOCKET_CLIENT_H
#define WEBSOCKET_CLIENT_H

#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>

#include "audio_packet.h"

esp_err_t websocket_client_init(const char *url);

esp_err_t websocket_send_text(const char *message);

void websocket_client_stop(void);

bool websocket_client_is_connected(void);

/* Send complete audio packet as binary WebSocket frame */
esp_err_t websocket_send_audio(
    const audio_packet_t *packet
);

#endif
