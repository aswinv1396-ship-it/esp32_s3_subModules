#include "ap_manager.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_mac.h"

static const char *TAG = "AP_MANAGER";

/* AP Network Interface */
static esp_netif_t *ap_netif = NULL;

/* Module Status */
static bool ap_initialized = false;

/* AP Running Status */
static bool ap_running = false;

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

static void ap_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id,  void *event_data)
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
            wifi_event_ap_staconnected_t *event =  (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "Client Connected : " MACSTR,  MAC2STR(event->mac));
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED:
        {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(TAG,"Client Disconnected : " MACSTR,MAC2STR(event->mac));
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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,  ESP_EVENT_ANY_ID,&ap_event_handler, NULL));
    ap_initialized = true;
    ESP_LOGI(TAG, "AP Manager Initialized");
    return ESP_OK;
}

/*-----------------------------------------------------------
 * Start Access Point
 *----------------------------------------------------------*/
esp_err_t ap_manager_start(void)
{
    esp_err_t ret;

    if (!ap_initialized)
    {
        ESP_LOGE(TAG, "AP Manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "Starting AP Manager");
    ESP_LOGI(TAG, "==================================================");

    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.ap.ssid, current_config.ssid,  sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char *)wifi_config.ap.password,current_config.password, sizeof(wifi_config.ap.password) - 1);
    
    ESP_LOGI(TAG, "After copy:");
	ESP_LOGI(TAG, "wifi_config.ap.ssid='%s'",  (char *)wifi_config.ap.ssid);
	ESP_LOGI(TAG, "wifi_config.ap.password='%s'",   (char *)wifi_config.ap.password);

    wifi_config.ap.ssid_len       = strlen(current_config.ssid);
    wifi_config.ap.channel        = current_config.channel;
    wifi_config.ap.max_connection = current_config.max_connections;
    wifi_config.ap.authmode = strlen(current_config.password) ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

    wifi_config.ap.pmf_cfg.required = false;

    ESP_LOGI(TAG, "AP Configuration");
    ESP_LOGI(TAG, "  SSID        : %s", wifi_config.ap.ssid);
    ESP_LOGI(TAG, "  PASSWORD    : %s", wifi_config.ap.password);
    ESP_LOGI(TAG, "  CHANNEL     : %d", wifi_config.ap.channel);
    ESP_LOGI(TAG, "  MAX CLIENTS : %d", wifi_config.ap.max_connection);
    ESP_LOGI(TAG, "  AUTHMODE    : %d", wifi_config.ap.authmode);

    //------------------------------------------------------
    // Current mode
    //------------------------------------------------------

    wifi_mode_t mode;

    ret = esp_wifi_get_mode(&mode);
    ESP_LOGI(TAG, "esp_wifi_get_mode() = %s (%d)",
             esp_err_to_name(ret),    mode);

    //------------------------------------------------------
    // Set APSTA mode
    //------------------------------------------------------

    ret = esp_wifi_set_mode(WIFI_MODE_APSTA);

    ESP_LOGI(TAG, "esp_wifi_set_mode(APSTA) = %s",
             esp_err_to_name(ret));

    ESP_ERROR_CHECK(ret);

    ret = esp_wifi_get_mode(&mode);

    ESP_LOGI(TAG, "Current mode after set_mode = %d",
             mode);


    //------------------------------------------------------
    // Verify mode after stop
    //------------------------------------------------------

    ret = esp_wifi_get_mode(&mode);

    ESP_LOGI(TAG,
             "Mode after stop = %d",
             mode);

    //------------------------------------------------------
    // Configure AP
    //------------------------------------------------------

    ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    ESP_LOGI(TAG,  "esp_wifi_set_config(AP) = %s",
             esp_err_to_name(ret));

    ESP_ERROR_CHECK(ret);

    //------------------------------------------------------
    // Read AP config back
    //------------------------------------------------------

    wifi_config_t verify_ap = {0};

    ret = esp_wifi_get_config(WIFI_IF_AP, &verify_ap);

    ESP_LOGI(TAG,
             "esp_wifi_get_config(AP) = %s",
             esp_err_to_name(ret));

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Verified AP Config");
        ESP_LOGI(TAG, "  SSID     : %s", verify_ap.ap.ssid);
        ESP_LOGI(TAG, "  PASSWORD : %s", verify_ap.ap.password);
        ESP_LOGI(TAG, "  CHANNEL  : %d", verify_ap.ap.channel);
    }

    //------------------------------------------------------
    // Read STA config
    //------------------------------------------------------

    wifi_config_t verify_sta = {0};

    ret = esp_wifi_get_config(WIFI_IF_STA, &verify_sta);

    ESP_LOGI(TAG,
             "esp_wifi_get_config(STA) = %s",
             esp_err_to_name(ret));

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Current STA Config");
        ESP_LOGI(TAG, "  SSID     : '%s'", verify_sta.sta.ssid);
        ESP_LOGI(TAG, "  PASSWORD : '%s'", verify_sta.sta.password);
        ESP_LOGI(TAG, "  SSID LEN : %u",
                 (unsigned)strlen((char *)verify_sta.sta.ssid));
        ESP_LOGI(TAG, "  PASS LEN : %u",
                 (unsigned)strlen((char *)verify_sta.sta.password));
    }

    //------------------------------------------------------
    // Final mode
    //------------------------------------------------------

    ret = esp_wifi_get_mode(&mode);

    ESP_LOGI(TAG,
             "Final WiFi mode = %d",
             mode);

    //------------------------------------------------------
    // AP IP
    //------------------------------------------------------

    char ip[16];

    ret = ap_manager_get_ip(ip, sizeof(ip));

    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "AP IP Address : %s", ip);
    }
    else
    {
        ESP_LOGE(TAG,
                 "Failed to get AP IP : %s",
                 esp_err_to_name(ret));
    }

    ESP_LOGI(TAG, "==================================================");
    ESP_LOGI(TAG, "AP Manager Started");
    ESP_LOGI(TAG, "==================================================");

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

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

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
    esp_err_t ret = esp_netif_get_ip_info( ap_netif, &ip_info);

    if (ret != ESP_OK)
    {
        return ret;
    }

    snprintf(ip,len,IPSTR,IP2STR(&ip_info.ip));
    return ESP_OK;
}
