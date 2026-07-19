#ifndef AP_MANAGER_H
#define AP_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

/*-----------------------------------------------------------
 * Default AP Configuration
 *----------------------------------------------------------*/

#define AP_DEFAULT_SSID             "ESP32-Assistant"
#define AP_DEFAULT_PASSWORD         "12345678"
#define AP_DEFAULT_CHANNEL          1
#define AP_DEFAULT_MAX_CONNECTIONS  4

/*-----------------------------------------------------------
 * AP Configuration Structure
 *----------------------------------------------------------*/

typedef struct
{
    char ssid[32];
    char password[64];
    uint8_t channel;
    uint8_t max_connections;

} ap_manager_config_t;

/*-----------------------------------------------------------
 * Public Functions
 *----------------------------------------------------------*/

esp_err_t ap_manager_init(void);

esp_err_t ap_manager_start(void);

esp_err_t ap_manager_stop(void);

bool ap_manager_is_running(void);

esp_err_t ap_manager_get_ip(char *ip, size_t len);

#endif