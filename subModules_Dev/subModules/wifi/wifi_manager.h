#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H


#include "esp_err.h"


#define WIFI_MAX_AP 20


typedef struct
{
    char ssid[33];
    int rssi;
    uint8_t channel;

} wifi_ap_info_t;



esp_err_t wifi_manager_init(void);


int wifi_manager_scan(
        wifi_ap_info_t *aps,
        int max_count
);


esp_err_t wifi_manager_connect(
        const char *ssid,
        const char *password
);


bool wifi_manager_is_connected(void);


#endif
