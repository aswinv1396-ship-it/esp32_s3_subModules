#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "esp_err.h"

#define WIFI_MAX_AP 20

typedef struct
{
    char ssid[33];
    int rssi;
    uint8_t channel;

} wifi_ap_info_t;

typedef struct
{
    char ssid[33];
    char password[64];
    bool valid;
} saved_wifi_t;

static saved_wifi_t saved_wifi =
{
    .ssid = "test",
    .password = "1234567v",
    .valid = true
};

/**
 * @brief Initialize the WiFi manager.
 *
 * This function initializes:
 *  - TCP/IP stack
 *  - Event loop
 *  - WiFi driver
 *  - WiFi station interface
 *
 * @return ESP_OK on success.
 */
esp_err_t wifi_manager_init(void);


/**
 * @brief Scan available WiFi access points.
 *
 * @param aps Pointer to output array.
 * @param max_count Maximum number of APs to store.
 *
 * @return Number of APs found.
 */
int wifi_manager_scan( wifi_ap_info_t *aps,int max_count);


/**
 * @brief Connect to a WiFi network.
 *
 * @param ssid WiFi SSID.
 * @param password WiFi password.
 *
 * @return ESP_OK if connected.
 */
esp_err_t wifi_manager_connect(const char *ssid, const char *password);

/**
 * @brief Check WiFi connection status.
 *
 * @return true if connected.
 * @return false otherwise.
 */
bool wifi_manager_is_connected(void);

esp_err_t wifi_manager_get_ip( char *ip, size_t len);
        
/**
 * @brief Deinitialize the WiFi manager.
 *
 * Stops WiFi, deinitializes the driver,
 * and releases allocated resources.
 *
 * @return ESP_OK on success.
 */
esp_err_t wifi_manager_deinit(void);


#ifdef __cplusplus
}
#endif

#endif /* WIFI_MANAGER_H */
