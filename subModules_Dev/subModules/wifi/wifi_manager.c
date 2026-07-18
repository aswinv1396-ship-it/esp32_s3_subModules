#include "wifi_manager.h"

#include <string.h>
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "wifi_manager";

static EventGroupHandle_t wifi_event_group;

static esp_netif_t *wifi_sta_netif = NULL;

static bool wifi_initialized = false;
static bool wifi_connected = false;

/*
 * WiFi event handler
 */
static void wifi_event_handler( void *arg,  esp_event_base_t event_base,int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT)
    {
        switch(event_id)
        {

        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG,"WiFi started");
            break;

		case WIFI_EVENT_STA_DISCONNECTED:
    		ESP_LOGW(TAG,"WiFi disconnected, retrying...");
		    wifi_connected = false;
		    xEventGroupClearBits( wifi_event_group,  WIFI_CONNECTED_BIT);
		    esp_wifi_connect();
		    break;

        default:
            break;
        }

    }


    if(event_base == IP_EVENT &&  event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi_connected = true;
        xEventGroupSetBits( wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/*
 * Initialize WiFi Manager
 */
esp_err_t wifi_manager_init(void)
{
    if(wifi_initialized)
        return ESP_OK;
    esp_err_t ret;

    /*
     * Initialize NVS
     */
    ret = nvs_flash_init();

    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        ret = nvs_flash_init();
    }

    if(ret != ESP_OK)
        return ret;

    /*
     * TCP/IP stack
     */
    ret = esp_netif_init();

    if(ret != ESP_OK)
        return ret;

    /*
     * Default event loop
     */
    ret = esp_event_loop_create_default();

    if(ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
        return ret;

    /*
     * Create station interface
     */
    wifi_sta_netif = esp_netif_create_default_wifi_sta();

    if(wifi_sta_netif == NULL)
        return ESP_FAIL;
    /*
     * Event group
     */
    wifi_event_group =    xEventGroupCreate();
    /*
     * WiFi driver init
     */

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ret = esp_wifi_init(&cfg);

    if(ret != ESP_OK)
        return ret;

    /*
     * Register events
     */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;


    esp_event_handler_instance_register( WIFI_EVENT,  ESP_EVENT_ANY_ID, &wifi_event_handler,  NULL,  &instance_any_id);

    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler,  NULL,  &instance_got_ip);

    /*
     * Set station mode
     */

    ret = esp_wifi_set_mode( WIFI_MODE_STA);

    if(ret != ESP_OK)
        return ret;
    /*
     * Start WiFi
     */

    ret = esp_wifi_start();

    if(ret != ESP_OK)
        return ret;

    wifi_initialized = true;
    ESP_LOGI(TAG, "WiFi manager initialized");

    return ESP_OK;
}

/*
 * Scan WiFi networks
 */
int wifi_manager_scan(wifi_ap_info_t *aps,int max_count)
{

    if(!wifi_initialized)
        return -1;

    if(aps == NULL || max_count <=0)
        return -1;

    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = false,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE
    };

    esp_err_t ret;

    ret = esp_wifi_scan_start( &scan_config,  true);

    if(ret != ESP_OK)
        return -1;

    uint16_t number = WIFI_MAX_AP;

    if(number > max_count)
        number = max_count;

    wifi_ap_record_t records[WIFI_MAX_AP];

    ret = esp_wifi_scan_get_ap_records(&number, records);

    if(ret != ESP_OK)
        return -1;

    for(int i=0;i<number;i++)
    {
        strncpy(aps[i].ssid, (char *)records[i].ssid, sizeof(aps[i].ssid)-1);

        aps[i].ssid[sizeof(aps[i].ssid)-1]=0;

        aps[i].rssi = records[i].rssi;

        aps[i].channel = records[i].primary;
    }

    return number;
}



/*
 * Connect to WiFi
 */
esp_err_t wifi_manager_connect( const char *ssid,const char *password)
{

    if(!wifi_initialized)
        return ESP_ERR_WIFI_NOT_INIT;

    if(ssid == NULL)
        return ESP_ERR_INVALID_ARG;

    wifi_config_t wifi_config = {0};

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));


    if(password)
    {
        strncpy((char *)wifi_config.sta.password,  password, sizeof(wifi_config.sta.password));
    }

    esp_err_t ret;

    ret = esp_wifi_set_config( WIFI_IF_STA, &wifi_config);

    if(ret != ESP_OK)
        return ret;

    ret = esp_wifi_connect();

    if(ret != ESP_OK)
        return ret;

    EventBits_t bits;

    bits = xEventGroupWaitBits( wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(15000));

    if(bits & WIFI_CONNECTED_BIT)
        return ESP_OK;
    return ESP_FAIL;
}

/*
 * Check connection
 */
bool wifi_manager_is_connected(void)
{
    return wifi_connected;
}

/*
 * Get local IP address
 */
esp_err_t wifi_manager_get_ip(char *ip,size_t len)
{

    if(ip == NULL || len == 0)
        return ESP_ERR_INVALID_ARG;

    esp_netif_ip_info_t ip_info;

    esp_err_t ret = esp_netif_get_ip_info(wifi_sta_netif, &ip_info);

    if(ret != ESP_OK)
        return ret;

    snprintf(ip, len, IPSTR, IP2STR(&ip_info.ip));

    return ESP_OK;
}

/*
 * Deinitialize WiFi Manager
 */
esp_err_t wifi_manager_deinit(void)
{

    if(!wifi_initialized)
        return ESP_OK;

    esp_wifi_stop();

    esp_wifi_deinit();

    if(wifi_sta_netif)
    {
        esp_netif_destroy( wifi_sta_netif);

        wifi_sta_netif = NULL;
    }

    if(wifi_event_group)
    {
        vEventGroupDelete( wifi_event_group);

        wifi_event_group=NULL;
    }

    wifi_connected=false;
    wifi_initialized=false;

    ESP_LOGI(TAG, "WiFi manager stopped");

    return ESP_OK;
}
