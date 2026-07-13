#include <string.h>


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_mac.h"


#include "nvs_flash.h"


#include "wifi_manager.h"



#define TAG "WIFI_MANAGER"


#define WIFI_CONNECTED_BIT BIT0



static EventGroupHandle_t wifi_event_group;



static void wifi_event_handler(
        void *arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void *event_data)
{


    if(event_base == WIFI_EVENT &&
       event_id == WIFI_EVENT_STA_START)
    {

        ESP_LOGI(TAG,
        "EVENT WIFI START");

    }



    else if(event_base == WIFI_EVENT &&
            event_id == WIFI_EVENT_STA_DISCONNECTED)
    {

        ESP_LOGW(TAG,
        "EVENT DISCONNECTED");


        xEventGroupClearBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT
        );

    }



    else if(event_base == IP_EVENT &&
            event_id == IP_EVENT_STA_GOT_IP)
    {


        ESP_LOGI(TAG,
        "EVENT GOT IP");


        xEventGroupSetBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT
        );

    }


}





esp_err_t wifi_manager_init(void)
{

    esp_err_t ret;



    ESP_LOGI(TAG,
    "[1] Event group START");


    wifi_event_group =
        xEventGroupCreate();


    if(!wifi_event_group)
    {
        ESP_LOGE(TAG,
        "Event group failed");

        return ESP_FAIL;
    }


    ESP_LOGI(TAG,
    "[1] Event group OK");



    ESP_LOGI(TAG,
    "[2] netif START");


    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    ESP_LOGI(TAG,
    "[2] netif OK");



    ESP_LOGI(TAG,
    "[3] event loop START");


    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    ESP_LOGI(TAG,
    "[3] event loop OK");



    esp_netif_create_default_wifi_sta();



    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();



    ESP_LOGI(TAG,
    "[4] wifi init START");



    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    ESP_LOGI(TAG,
    "[4] wifi init OK");




    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            wifi_event_handler,
            NULL
        )
    );



    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            wifi_event_handler,
            NULL
        )
    );



    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA
        )
    );



    ESP_LOGI(TAG,
    "[5] wifi start START");



    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(TAG,
    "[5] wifi start OK");



    ESP_LOGI(TAG,
    "========== WIFI READY ==========");


    return ESP_OK;

}







int wifi_manager_scan(
        wifi_ap_info_t *aps,
        int max_count)
{


    ESP_LOGI(TAG,
    "Starting WiFi scan");



    wifi_scan_config_t scan =
    {
        .ssid=NULL,
        .bssid=NULL,
        .channel=0,
        .show_hidden=false
    };



    ESP_ERROR_CHECK(
        esp_wifi_scan_start(
            &scan,
            true
        )
    );



    uint16_t count=WIFI_MAX_AP;



    wifi_ap_record_t records[WIFI_MAX_AP];



    ESP_ERROR_CHECK(
        esp_wifi_scan_get_ap_records(
            &count,
            records
        )
    );



    if(count>max_count)
        count=max_count;




    for(int i=0;i<count;i++)
    {

        strcpy(
            aps[i].ssid,
            (char *)records[i].ssid
        );


        aps[i].rssi =
            records[i].rssi;


        aps[i].channel =
            records[i].primary;


    }


    ESP_LOGI(TAG,
    "Scan complete : %d networks",
    count);



    return count;

}









esp_err_t wifi_manager_connect(
        const char *ssid,
        const char *password)
{


    wifi_config_t wifi_config={0};



    strcpy(
        (char *)wifi_config.sta.ssid,
        ssid
    );


    strcpy(
        (char *)wifi_config.sta.password,
        password
    );



    ESP_LOGI(TAG,
    "Connecting SSID: %s",
    ssid);



    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );



    ESP_ERROR_CHECK(
        esp_wifi_connect()
    );



    EventBits_t bits;



    bits=xEventGroupWaitBits(
        wifi_event_group,
        WIFI_CONNECTED_BIT,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(30000)
    );



    if(bits & WIFI_CONNECTED_BIT)
    {

        ESP_LOGI(TAG,
        "CONNECTED");


        return ESP_OK;

    }



    ESP_LOGE(TAG,
    "CONNECT FAILED");


    return ESP_FAIL;

}






bool wifi_manager_is_connected(void)
{

    EventBits_t bits;


    bits=xEventGroupGetBits(
        wifi_event_group
    );


    return
    (bits & WIFI_CONNECTED_BIT);

}

