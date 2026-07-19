#include "ap_manager.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"
/*-----------------------------------------------------------
 * Private Definitions
 *----------------------------------------------------------*/

static const char *TAG = "AP_MANAGER";

/*-----------------------------------------------------------
 * Private Variables
 *----------------------------------------------------------*/

/* AP Network Interface */
static esp_netif_t *ap_netif = NULL;

/* Module Status */
static bool ap_initialized = false;

/* AP Running Status */
static bool ap_running = false;

/*
 * Default AP Configuration
 */
static ap_manager_config_t current_config =
{
    .ssid            = AP_DEFAULT_SSID,
    .password        = AP_DEFAULT_PASSWORD,
    .channel         = AP_DEFAULT_CHANNEL,
    .max_connections = AP_DEFAULT_MAX_CONNECTIONS
};

/*-----------------------------------------------------------
 * AP Event Handler
 *----------------------------------------------------------*/

static void ap_event_handler(void *arg,
                             esp_event_base_t event_base,
                             int32_t event_id,
                             void *event_data)
{
    if (event_base != WIFI_EVENT)
    {
        return;
    }

    switch (event_id)
    {
        case WIFI_EVENT_AP_START:

            ap_running = true;

            ESP_LOGI(TAG, "Access Point Started");

            break;

        case WIFI_EVENT_AP_STOP:

            ap_running = false;

            ESP_LOGI(TAG, "Access Point Stopped");

            break;

        case WIFI_EVENT_AP_STACONNECTED:
        {
            wifi_event_ap_staconnected_t *event =
                (wifi_event_ap_staconnected_t *)event_data;

            ESP_LOGI(TAG,
                     "Client Connected : " MACSTR,
                     MAC2STR(event->mac));

            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *event =
                (wifi_event_ap_stadisconnected_t *)event_data;

            ESP_LOGI(TAG,
                     "Client Disconnected : " MACSTR,
                     MAC2STR(event->mac));

            break;
        }

        default:
            break;
    }
}

/*-----------------------------------------------------------
 * Initialize AP Manager
 *----------------------------------------------------------*/

esp_err_t ap_manager_init(void)
{
    if (ap_initialized)
    {
        ESP_LOGW(TAG, "AP Manager already initialized");

        return ESP_OK;
    }

    /*
     * Create AP Network Interface
     *
     * WiFi driver must already be initialized
     * by wifi_manager_init().
     */
    ap_netif = esp_netif_create_default_wifi_ap();

    if (ap_netif == NULL)
    {
        ESP_LOGE(TAG, "Failed to create AP interface");

        return ESP_FAIL;
    }

    /*
     * Register AP Event Handler
     */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &ap_event_handler,
            NULL));

    ap_initialized = true;

    ESP_LOGI(TAG, "AP Manager Initialized");

    return ESP_OK;
}

/*-----------------------------------------------------------
 * Start Access Point
 *----------------------------------------------------------*/

esp_err_t ap_manager_start(void)
{
    if (!ap_initialized)
    {
        ESP_LOGE(TAG, "AP Manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.ap.ssid,
            current_config.ssid,
            sizeof(wifi_config.ap.ssid));

    strncpy((char *)wifi_config.ap.password,
            current_config.password,
            sizeof(wifi_config.ap.password));

    wifi_config.ap.ssid_len = strlen(current_config.ssid);

    wifi_config.ap.channel = current_config.channel;

    wifi_config.ap.max_connection = current_config.max_connections;

    wifi_config.ap.authmode =
        (strlen(current_config.password) > 0) ?
        WIFI_AUTH_WPA2_PSK :
        WIFI_AUTH_OPEN;

    wifi_config.ap.pmf_cfg.required = false;

    /*
     * Enable AP + Station Mode
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_APSTA));

    /*
     * Configure AP
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_AP,
            &wifi_config));

    /*
     * WiFi may already be running from
     * wifi_manager_init().
     */
    esp_err_t ret = esp_wifi_start();

    if (ret != ESP_OK &&
        ret != ESP_ERR_WIFI_CONN &&
        ret != ESP_ERR_WIFI_NOT_STOPPED)
    {
        ESP_LOGE(TAG,
                 "esp_wifi_start() failed : %s",
                 esp_err_to_name(ret));

        return ret;
    }

    ESP_LOGI(TAG, "=================================");
    ESP_LOGI(TAG, "Access Point Started");
    ESP_LOGI(TAG, "SSID     : %s", current_config.ssid);
    ESP_LOGI(TAG, "Password : %s", current_config.password);
    ESP_LOGI(TAG, "Channel  : %d", current_config.channel);
    ESP_LOGI(TAG, "Clients  : %d", current_config.max_connections);
    ESP_LOGI(TAG, "=================================");

    char ip[16];

    if (ap_manager_get_ip(ip, sizeof(ip)) == ESP_OK)
    {
        ESP_LOGI(TAG, "AP IP Address : %s", ip);
    }

    return ESP_OK;
}

/*-----------------------------------------------------------
 * Stop Access Point
 *----------------------------------------------------------*/

esp_err_t ap_manager_stop(void)
{
    if (!ap_initialized)
    {
        return ESP_OK;
    }

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA));

    ap_running = false;

    ESP_LOGI(TAG, "Access Point Stopped");

    return ESP_OK;
}

/*-----------------------------------------------------------
 * AP Running Status
 *----------------------------------------------------------*/

bool ap_manager_is_running(void)
{
    return ap_running;
}

/*-----------------------------------------------------------
 * Get AP IP Address
 *----------------------------------------------------------*/

esp_err_t ap_manager_get_ip(char *ip, size_t len)
{
    if (ip == NULL || len == 0)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (ap_netif == NULL)
    {
        return ESP_FAIL;
    }

    esp_netif_ip_info_t ip_info;

    esp_err_t ret =
        esp_netif_get_ip_info(
            ap_netif,
            &ip_info);

    if (ret != ESP_OK)
    {
        return ret;
    }

    snprintf(ip,
             len,
             IPSTR,
             IP2STR(&ip_info.ip));

    return ESP_OK;
}