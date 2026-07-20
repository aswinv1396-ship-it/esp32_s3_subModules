#include "wifi_manager.h"

#include <string.h>
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define WIFI_CONNECTED_BIT BIT0

static const char *TAG = "wifi_manager";

static EventGroupHandle_t wifi_event_group = NULL;

static esp_netif_t *wifi_sta_netif = NULL;
/*
saved_wifi_t saved_wifi =
{
    .ssid = "test",
    .password = "1234567v",
    .valid = true
};
*/
// Initialize the 5-slot history array to zeroes
saved_wifi_t saved_wifi[WIFI_HISTORY_MAX] = {0};

static char selected_ssid[33]={0};
static char current_password[64]={0};

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
			wifi_event_sta_disconnected_t *e = event_data;
    		ESP_LOGW(TAG,"WiFi disconnected, reason=%d",  e->reason);
		    wifi_connected = false;
		    xEventGroupClearBits( wifi_event_group,  WIFI_CONNECTED_BIT);
		    //esp_wifi_connect();
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
 
esp_err_t wifi_manager_connect(const char *ssid, const char *password)
{
    esp_err_t ret;

    ESP_LOGI(TAG, "=================================================");
    ESP_LOGI(TAG, "wifi_manager_connect()");
    ESP_LOGI(TAG, "=================================================");

    /*-------------------------------------------------------
     * Check WiFi manager
     *------------------------------------------------------*/
    if (!wifi_initialized)
    {
        ESP_LOGE(TAG, "WiFi manager not initialized");
        return ESP_ERR_WIFI_NOT_INIT;
    }

    /*-------------------------------------------------------
     * Check input pointers
     *------------------------------------------------------*/
    if (ssid == NULL || password == NULL)
    {
        ESP_LOGE(TAG, "SSID or Password is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    /*-------------------------------------------------------
     * Calculate input lengths
     *------------------------------------------------------*/
    size_t ssid_len = strlen(ssid);
    size_t password_len = strlen(password);

    ESP_LOGI(TAG, "Input SSID     : '%s'", ssid);
    ESP_LOGI(TAG, "Input PASSWORD : '%s'", password);
    ESP_LOGI(TAG, "SSID Length    : %u", (unsigned)ssid_len);
    ESP_LOGI(TAG, "PASS Length    : %u", (unsigned)password_len);

    if (ssid_len == 0 || ssid_len > 32)
    {
        ESP_LOGE(TAG, "Invalid SSID length: %u", (unsigned)ssid_len);
        return ESP_ERR_INVALID_ARG;
    }

    if (password_len > 63)
    {
        ESP_LOGE(TAG, "Invalid password length: %u", (unsigned)password_len);
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "SSID Bytes:");
    for (size_t i = 0; i < ssid_len; i++)
    {
        ESP_LOGI(TAG, "  [%02u] = 0x%02X ('%c')", (unsigned)i, (unsigned char)ssid[i], ssid[i]);
    }

    ESP_LOGI(TAG, "Password Bytes:");
    for (size_t i = 0; i < password_len; i++)
    {
        ESP_LOGI(TAG, "  [%02u] = 0x%02X", (unsigned)i, (unsigned char)password[i]);
    }

    /*-------------------------------------------------------
     * FIX: Allocate temporary stack buffers.
     * This shields the data from being zeroed out if 'ssid' 
     * or 'password' overlap with the global memory spaces.
     *------------------------------------------------------*/
    char tmp_ssid[33] = {0};
    char tmp_pass[64] = {0};
    
    memcpy(tmp_ssid, ssid, ssid_len);
    memcpy(tmp_pass, password, password_len);

    /*-------------------------------------------------------
     * Safely update internal global variables
     *------------------------------------------------------*/
    memset(selected_ssid, 0, sizeof(selected_ssid));
    memcpy(selected_ssid, tmp_ssid, ssid_len);

    memset(current_password, 0, sizeof(current_password));
    memcpy(current_password, tmp_pass, password_len);

    ESP_LOGI(TAG, "Credentials stored internally");
    ESP_LOGI(TAG, "selected_ssid='%s'", selected_ssid);
    ESP_LOGI(TAG, "current_password='%s'", current_password);

    /*-------------------------------------------------------
     * Prepare STA Configuration using local buffers
     *------------------------------------------------------*/
    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));

    memcpy(wifi_config.sta.ssid, tmp_ssid, ssid_len);
    memcpy(wifi_config.sta.password, tmp_pass, password_len);

    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_LOGI(TAG, "Prepared STA Configuration");
    ESP_LOGI(TAG, "SSID='%s'", (char *)wifi_config.sta.ssid);
    ESP_LOGI(TAG, "PASS='%s'", (char *)wifi_config.sta.password);
    ESP_LOGI(TAG, "SSID Length=%u", (unsigned)strlen((char *)wifi_config.sta.ssid));
    ESP_LOGI(TAG, "PASS Length=%u", (unsigned)strlen((char *)wifi_config.sta.password));

    ESP_LOGI(TAG, "SSID raw bytes:");
    for (int i = 0; i < 32; i++)
    {
        ESP_LOGI(TAG, "SSID[%02d] = 0x%02X", i, wifi_config.sta.ssid[i]);
    }

    /*-------------------------------------------------------
     * Apply configuration
     *------------------------------------------------------*/
    ret = esp_wifi_disconnect();
    if (ret != ESP_OK && ret != ESP_ERR_WIFI_NOT_CONNECT)
    {
        ESP_LOGW(TAG, "esp_wifi_disconnect() returned %s", esp_err_to_name(ret));
    }

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    ESP_LOGI(TAG, "esp_wifi_set_config() = %s (%d)", esp_err_to_name(ret), ret);
    if (ret != ESP_OK)
    {
        return ret;
    }

    /*-------------------------------------------------------
     * Verify Configuration
     *------------------------------------------------------*/
    wifi_config_t verify;
    memset(&verify, 0, sizeof(verify));

    ret = esp_wifi_get_config(WIFI_IF_STA, &verify);
    ESP_LOGI(TAG, "esp_wifi_get_config() = %s (%d)", esp_err_to_name(ret), ret);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ESP_LOGI(TAG, "Driver Configuration");
    ESP_LOGI(TAG, "SSID='%s'", (char *)verify.sta.ssid);
    ESP_LOGI(TAG, "PASS='%s'", (char *)verify.sta.password);
    ESP_LOGI(TAG, "SSID Length=%u", (unsigned)strlen((char *)verify.sta.ssid));
    ESP_LOGI(TAG, "PASS Length=%u", (unsigned)strlen((char *)verify.sta.password));

    if (strlen((char *)verify.sta.ssid) == 0)
    {
        ESP_LOGE(TAG, "ERROR: Driver STA SSID is empty after set_config");
        return ESP_FAIL;
    }

    /*-------------------------------------------------------
     * Reset connection state
     *------------------------------------------------------*/
    wifi_connected = false;
    xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);

    /*-------------------------------------------------------
     * Connect
     *------------------------------------------------------*/
    ESP_LOGI(TAG, "Calling esp_wifi_connect()...");
    ret = esp_wifi_connect();
    ESP_LOGI(TAG, "esp_wifi_connect() returned %s (%d)", esp_err_to_name(ret), ret);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "esp_wifi_connect() failed immediately");
        return ret;
    }

    ESP_LOGI(TAG, "Waiting for IP Event...");
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT,
                                           pdFALSE,
                                           pdTRUE,
                                           pdMS_TO_TICKS(20000));

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "WiFi Connected Successfully");
        return ESP_OK;
    }

    ESP_LOGE(TAG, "Timed out waiting for IP address");
    return ESP_FAIL;
}


/*
 * Check connection
 */
bool wifi_manager_is_connected(void)
{
    return wifi_connected;
}

void wifi_manager_set_selected_ssid(char *ssid)
{
    if(ssid == NULL)
        return;

    memset( selected_ssid, 0,sizeof(selected_ssid) );
    strncpy( selected_ssid,  ssid, sizeof(selected_ssid)-1 );

    ESP_LOGI( "WIFI_MANAGER",  "Selected SSID=%s",  selected_ssid );

}

char *wifi_manager_get_selected_ssid(void)
{
    return selected_ssid;
}

/*-------------------------------------------------------
 * Helper Function: Manages history array shifting
 *------------------------------------------------------*/
void wifi_manager_add_to_history(const char *ssid, const char *password)
{
    if (ssid == NULL || strlen(ssid) == 0) return;

    int target_index = -1;

    // 1. Look for existing network entries to avoid duplicates
    for (int i = 0; i < WIFI_HISTORY_MAX; i++) {
        if (saved_wifi[i].valid && strcmp(saved_wifi[i].ssid, ssid) == 0) {
            target_index = i;
            break;
        }
    }

    // 2. Duplicate found: Extract its info, shift elements up, and reset it to slot 0
    if (target_index != -1) {
        saved_wifi_t existing = saved_wifi[target_index];
        if (password != NULL) {
            strncpy(existing.password, password, sizeof(existing.password) - 1);
        }
        for (int i = target_index; i > 0; i--) {
            saved_wifi[i] = saved_wifi[i - 1];
        }
        saved_wifi[0] = existing;
        return;
    }

    // 3. New Network found: Drop the 5th network, shift others down to clear slot 0
    for (int i = WIFI_HISTORY_MAX - 1; i > 0; i--) {
        saved_wifi[i] = saved_wifi[i - 1];
    }

    // 4. Populate slot 0 with the newest configuration
    memset(&saved_wifi[0], 0, sizeof(saved_wifi_t));
    strncpy(saved_wifi[0].ssid, ssid, sizeof(saved_wifi[0].ssid) - 1);
    if (password != NULL) {
        strncpy(saved_wifi[0].password, password, sizeof(saved_wifi[0].password) - 1);
    }
    saved_wifi[0].valid = true;
    
    ESP_LOGI(TAG, "Network '%s' added to history index", ssid);
}

/*-------------------------------------------------------
 * Updated: Stores entire history array into NVS Flash
 *------------------------------------------------------*/
esp_err_t wifi_manager_save_credentials(void)
{
    nvs_handle_t nvs;

    // First update the internal runtime history array layout
    wifi_manager_add_to_history(selected_ssid, current_password);

    esp_err_t ret = nvs_open("wifi", NVS_READWRITE, &nvs);
    if(ret != ESP_OK)
        return ret;

    // Save the entire 5-network profile configuration block to NVS flash memory
    ret = nvs_set_blob(nvs, "wifi_hist_blob", saved_wifi, sizeof(saved_wifi));
    if(ret != ESP_OK)
    {
        nvs_close(nvs);
        return ret;
    }
    
    ret = nvs_commit(nvs);
    nvs_close(nvs);
    
    if(ret == ESP_OK)
    {
        ESP_LOGI("WIFI_MANAGER", "Credentials profile history block successfully stored");
    }

    return ret;
}

/*-------------------------------------------------------
 * New Loader Function: Read history out of NVS on startup
 *------------------------------------------------------*/
esp_err_t wifi_manager_load_credentials(void)
{
    nvs_handle_t nvs;
    esp_err_t ret = nvs_open("wifi", NVS_READONLY, &nvs);
    if(ret != ESP_OK)
        return ret;

    size_t required_size = sizeof(saved_wifi);
    ret = nvs_get_blob(nvs, "wifi_hist_blob", saved_wifi, &required_size);
    
    nvs_close(nvs);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Loaded %d stored Wi-Fi history files from flash memory.", WIFI_HISTORY_MAX);
    }
    return ret;
}

/*-------------------------------------------------------
 * Scan and Auto-Connect Known Networks Loop
 *------------------------------------------------------*/
bool wifi_manager_scan_and_connect_known(wifi_ap_info_t *scanned_aps, int scanned_count)
{
    ESP_LOGI(TAG, "Parsing %d scanned networks against history profiles...", scanned_count);

    if (scanned_aps == NULL || scanned_count == 0) 
    {
        ESP_LOGW(TAG, "Scan results empty. Moving straight to portal config mode.");
        return false;
    }

    // Outer-loop checks structural history array positions sequentially (Slot 0 is highest priority)
    for (int h = 0; h < WIFI_HISTORY_MAX; h++) 
    {
        if (!saved_wifi[h].valid) 
            continue;

        // Inner-loop: Check if the current profile matches a live visible hotspot
        for (int s = 0; s < scanned_count; s++) 
        {
            if (strcmp(scanned_aps[s].ssid, saved_wifi[h].ssid) == 0) 
            {
                ESP_LOGI(TAG, "Match Found! Stored network '%s' is visible (RSSI: %d dBm)", 
                         saved_wifi[h].ssid, scanned_aps[s].rssi);
                
                // Configure local state properties context before firing initialization wrappers
                wifi_manager_set_selected_ssid(saved_wifi[h].ssid);
                wifi_manager_set_password(saved_wifi[h].password);

                // Attempt network connection sequence
                esp_err_t ret = wifi_manager_connect(saved_wifi[h].ssid, saved_wifi[h].password);

                if (ret == ESP_OK) 
                {
                    ESP_LOGI(TAG, "Successfully background connected to network: '%s'!", saved_wifi[h].ssid);
                    // Rearrange hierarchy stack to push this profile to position index 0
                    wifi_manager_save_credentials(); 
                    return true; 
                }

                // If connection drops or returns error, loop continues to seek next match profile
                ESP_LOGE(TAG, "Connection failed for '%s' (Wrong password or timeout). Trying next...", saved_wifi[h].ssid);
            }
        }
    }

    ESP_LOGW(TAG, "No historical networks could connect successfully. Remaining in portal configuration mode.");
    return false;
}

/*
 * Get local IP address
 */
esp_err_t wifi_manager_get_ip(char *ip, size_t len)
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

const char *wifi_manager_get_connected_ssid(void)
{
    return selected_ssid;
}

void wifi_manager_set_password(char *password)
{
    if(password == NULL)
        return;

    memset(current_password, 0, sizeof(current_password));
    strncpy(current_password, password, sizeof(current_password) - 1);
    ESP_LOGI("WIFI_MANAGER", "Password updated");
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
        esp_netif_destroy(wifi_sta_netif);
        wifi_sta_netif = NULL;
    }

    if(wifi_event_group)
    {
        vEventGroupDelete(wifi_event_group);
        wifi_event_group = NULL;
    }

    wifi_connected = false;
    wifi_initialized = false;

    ESP_LOGI(TAG, "WiFi manager stopped");
    return ESP_OK;
}
