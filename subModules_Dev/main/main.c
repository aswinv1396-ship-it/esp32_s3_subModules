#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "wifi_manager.h"
#include "console_manager.h"
#include "inmp441.h"


static const char *TAG = "MAIN";

#define MAX_ATTEMPTS 4

/*
 * Internet connectivity check
 */
static bool check_internet(void)
{
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ESP_LOGI(TAG, "Resolving google.com");

    int err = getaddrinfo("google.com","80",  &hints, &res);

    if(err != 0 || res == NULL)
    {
        ESP_LOGE(TAG, "DNS failed: %d",  err);
        return false;
    }

    int sock =socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(sock < 0)
    {
        ESP_LOGE(TAG, "Socket creation failed");
        freeaddrinfo(res);
        return false;
    }

    struct timeval timeout =
    {
        .tv_sec = 5,
        .tv_usec = 0
    };

    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    ESP_LOGI(TAG, "Connecting to google.com");

    if(connect(sock, res->ai_addr, res->ai_addrlen) != 0)
    {

        ESP_LOGE(TAG,  "Connection failed");
        close(sock);
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);

    const char *request =
        "GET / HTTP/1.1\r\n"
        "Host: google.com\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(sock, request, strlen(request),0);

    char buffer[128];

    int len =recv(sock, buffer, sizeof(buffer)-1, 0);

    close(sock);

    if(len <= 0)
    {
        ESP_LOGE(TAG,"No HTTP response");
        return false;
    }

    buffer[len] = '\0';
    ESP_LOGI(TAG, "HTTP response: %.40s", buffer);

    if(strstr(buffer,"HTTP/1.1") || strstr(buffer,"HTTP/1.0"))
    {
        return true;
    }

    return false;
}

static bool wifi_setup_process(void)
{
    wifi_ap_info_t aps[WIFI_MAX_AP];

    while(1)
    {

        console_print( "\nScanning WiFi networks...\n");

        int ap_count = wifi_manager_scan( aps,  WIFI_MAX_AP);

        if(ap_count <= 0)
        {
            console_print( "No WiFi networks found\n");
            return false;
        }

        console_print("\nAvailable WiFi Networks:\n");
        console_print( "============================\n");

        for(int i = 0; i < ap_count; i++)
        {

            console_print( "%d) SSID: %s  RSSI:%d  CH:%d\n", i + 1, aps[i].ssid, aps[i].rssi,aps[i].channel);

        }

        console_print(  "============================\n");

        int selection =  console_read_int(  "Select WiFi number: ");

        if(selection < 1 || selection > ap_count)
        {
            console_print(  "Invalid WiFi selection\n");
            continue;
        }

        int attempt = 0;
        /*
         * Password retry loop
         */
        while(attempt < MAX_ATTEMPTS)
        {
            char password[64];
            console_read_string(  "Enter password: ",  password,   sizeof(password));

            console_print( "\nConnecting to %s...\n",   aps[selection-1].ssid);

            esp_err_t ret =    wifi_manager_connect( aps[selection-1].ssid,   password);

            if(ret == ESP_OK &&  wifi_manager_is_connected())
            {

                console_print( "WiFi connected\n");

                char ip[16];
                if(wifi_manager_get_ip( ip,  sizeof(ip))  == ESP_OK)
                {
                    console_print("IP Address : %s\n",  ip);
                }
                /*
                 * Check internet
                 */
                if(check_internet())
                {

                    console_print( "Internet available\n");
                    return true;
                }
                else
                {
                    console_print( "WiFi connected but internet unavailable\n");
                }

            }
            else
            {
                console_print("Wrong password or connection failed\n");
            }

            attempt++;

            console_print(   "Attempt %d/%d failed\n",  attempt,MAX_ATTEMPTS);

            vTaskDelay(pdMS_TO_TICKS(1000));

        }

        /*
         * Three attempts completed
         */
        char answer[8];
        console_read_string( "\nMaximum attempts reached. Scan again? (y/n): ",  answer,sizeof(answer));

        if(answer[0] == 'y' || answer[0] == 'Y')
        {
            console_print(  "Restarting WiFi scan...\n");
            continue;
        }
        else
        {
            console_print( "Exiting WiFi configuration\n");
            return false;
        }
    }
}




void app_main(void)
{

    /*
     * Start USB console
     */

    console_manager_init();



    /*
     * Initialize WiFi
     */

    esp_err_t ret =
        wifi_manager_init();



    if(ret != ESP_OK)
    {

        ESP_LOGE(TAG,
                 "WiFi initialization failed");

        return;

    }



    ESP_LOGI(TAG,
             "WiFi manager ready");



    /*
     * Start user WiFi setup
     */

    if(wifi_setup_process())
    {

        ESP_LOGI(TAG,
                 "Network setup successful");

    }
    else
    {

        ESP_LOGE(TAG,
                 "Network setup stopped");

    }



    /*
     * Keep application alive
     */

    while(1)
    {

        vTaskDelay(
            pdMS_TO_TICKS(1000));

    }

}
