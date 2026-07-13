#include <stdio.h>


#include "esp_log.h"


#include "wifi_manager.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"


#define TAG "MAIN"



void app_main(void)
{


    ESP_LOGI(TAG,
    "========== WIFI TEST ==========");



    ESP_LOGI(TAG,
    "[1] WIFI INIT START");



    if(wifi_manager_init()!=ESP_OK)
    {

        ESP_LOGE(TAG,
        "WIFI INIT FAILED");

        return;

    }



    ESP_LOGI(TAG,
    "[1] WIFI INIT OK");





    wifi_ap_info_t aps[WIFI_MAX_AP];



    ESP_LOGI(TAG,
    "[2] SCAN START");



    int count=
        wifi_manager_scan(
            aps,
            WIFI_MAX_AP
        );



    if(count<=0)
    {

        ESP_LOGE(TAG,
        "NO NETWORK FOUND");

        return;

    }




    ESP_LOGI(TAG,
    "========== NETWORK LIST ==========");



    for(int i=0;i<count;i++)
    {

        printf(
        "%d : %s RSSI=%d CH=%d\n",
        i,
        aps[i].ssid,
        aps[i].rssi,
        aps[i].channel
        );

    }




    /*
       For now select manually

       Later this can come from:
       UART
       BLE
       Touch display
       Web page
    */


    int selected=0;



    char password[64];



    printf(
    "Select Network Number: "
    );



    scanf(
    "%d",
    &selected
    );



    printf(
    "Enter Password: "
    );


    scanf(
    "%63s",
    password
    );





    ESP_LOGI(TAG,
    "[3] CONNECT START");




    if(
    wifi_manager_connect(
        aps[selected].ssid,
        password
    )
    !=ESP_OK)
    {

        ESP_LOGE(TAG,
        "CONNECT FAILED");

        return;

    }





    ESP_LOGI(TAG,
    "========== SYSTEM RUNNING ==========");



    while(1)
    {

        if(wifi_manager_is_connected())
        {
            ESP_LOGI(TAG,
            "WiFi OK");
        }
        else
        {
            ESP_LOGW(TAG,
            "WiFi LOST");
        }


        vTaskDelay(
            pdMS_TO_TICKS(5000)
        );

    }


}
