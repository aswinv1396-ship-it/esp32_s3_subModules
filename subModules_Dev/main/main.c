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

#include "esp_netif_sntp.h"
#include "esp_log.h"
#include <time.h>

#include "wifi_manager.h"
#include "console_manager.h"
#include "inmp441.h"
#include "websocket_client.h"
#include "time_sync.h"
#include "webportal.h"
#include "ap_manager.h"

static const char *TAG = "MAIN";

#define MAX_ATTEMPTS 4
#define MIC_TASK_STACK_SIZE    4096
#define MIC_TASK_PRIORITY      5

static const char *TAG_SNTP = "SNTP";

static esp_err_t obtain_time(void)
{
    ESP_LOGI(TAG_SNTP, "Starting SNTP");

    /*
     * Configure SNTP using the correct v5.0 structure type
     */
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG( "pool.ntp.org");

    esp_err_t ret =   esp_netif_sntp_init(&config );

    if (ret != ESP_OK) { ESP_LOGE(TAG_SNTP, "SNTP initialization failed: %s", esp_err_to_name(ret)); return ret; }

    /*
     * Wait for synchronization
     */
    ESP_LOGI(TAG_SNTP, "Waiting for time synchronization...");

    ret =
        esp_netif_sntp_sync_wait(
            pdMS_TO_TICKS(15000)
        );

    if (ret != ESP_OK) { ESP_LOGE(TAG_SNTP, "SNTP synchronization failed: %s", esp_err_to_name(ret)); esp_netif_sntp_deinit(); return ret; }

    /*
     * Read synchronized POSIX time
     */
    time_t now = time(
        NULL
    );

    struct tm timeinfo;

    localtime_r(
        &now,
        &timeinfo
    );

    ESP_LOGI(TAG_SNTP, "Time synchronized successfully");

    ESP_LOGI(TAG_SNTP, "Current time: %04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    ESP_LOGI(TAG_SNTP, "Unix time: %lld", (long long)now);

    return ESP_OK;
}

/*-----------------------------------------------------------
 * POSIX Time Verification
 *----------------------------------------------------------*/

static esp_err_t verify_system_time(void)
{
    time_t now;

    struct tm timeinfo;

    /*
     * Test 1:
     * time(NULL)
     */
    now = time(NULL);

    localtime_r(
        &now,
        &timeinfo
    );

    ESP_LOGI("TIME", "time(NULL) = %lld", (long long)now);

    ESP_LOGI("TIME", "Current date = %04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    /*
     * Test 2:
     * gettimeofday()
     */
    struct timeval tv;

    if (gettimeofday(&tv, NULL) != 0) { ESP_LOGE("TIME", "gettimeofday() failed"); return ESP_FAIL; }

    ESP_LOGI("TIME", "gettimeofday.tv_sec = %lld", (long long)tv.tv_sec);

    /*
     * Compare the two clocks
     */
    long long difference = (long long)tv.tv_sec - (long long)now;

    if (difference < 0) difference = -difference;

    ESP_LOGI("TIME", "Clock difference = %lld seconds", difference);

    /*
     * Expected date check
     */
    if ((timeinfo.tm_year + 1900) < 2026) { ESP_LOGE("TIME", "System time is invalid"); return ESP_FAIL; }

    ESP_LOGI("TIME", "POSIX system time verified");

    return ESP_OK;
}

/*-----------------------------------------------------------
 * Memory Verification
 *----------------------------------------------------------*/

static void print_memory_status(void)
{
    ESP_LOGI("MEM", "Free heap: %lu bytes", (unsigned long)esp_get_free_heap_size());

    ESP_LOGI("MEM", "Minimum free heap: %lu bytes", (unsigned long)esp_get_minimum_free_heap_size());

    ESP_LOGI("MEM", "Largest internal free block: %lu bytes", (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
}

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

int find_saved_network(wifi_ap_info_t *aps, int ap_count)
{
    if (!saved_wifi.valid)
    {
        return -1;
    }

    for (int i = 0; i < ap_count; i++)
    {
        if (strcmp(saved_wifi.ssid, aps[i].ssid) == 0)
        {
            return i;
        }
    }

    return -1;
}

bool connect_saved_network(wifi_ap_info_t *ap)
{
    console_print("\nConnecting to saved network: %s\n", ap->ssid);

    esp_err_t ret = wifi_manager_connect(saved_wifi.ssid, saved_wifi.password);

    if (ret != ESP_OK || !wifi_manager_is_connected())
    {
        console_print("Saved credentials failed.\n");
        return false;
    }

    char ip[16];

    if (wifi_manager_get_ip(ip, sizeof(ip)) == ESP_OK)
    {
        console_print("IP Address: %s\n", ip);
    }

    if (check_internet())
    {
        console_print("Internet available.\n");
        return true;
    }

    console_print("Connected but internet unavailable.\n");

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

        int saved_index = find_saved_network(aps, ap_count);
        if (saved_index >= 0)
        {
            char ans[8];
            
            console_print("\nSaved network \"%s\" found.\n", saved_wifi.ssid);       
            console_read_string("Connect to saved network? (y/n): ", ans, sizeof(ans));
            
            if (ans[0] == 'y' || ans[0] == 'Y')
            {
                if (connect_saved_network(&aps[saved_index]))
                {
                    return true;
                }
                console_print("Unable to connect using saved credentials.\n");
            }
        }

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
    console_manager_init();

    esp_err_t ret =  wifi_manager_init();

    if (ret != ESP_OK) 
    { 
        ESP_LOGE(TAG, "WiFi initialization failed"); 
        return; 
    }

    ESP_LOGI(TAG, "WiFi manager ready");

    ESP_ERROR_CHECK(ap_manager_init());

    ESP_LOGI(TAG, "AP Manager Ready");

    ESP_ERROR_CHECK(ap_manager_start());

    ESP_LOGI(TAG, "Local Hotspot Ready");

    ESP_ERROR_CHECK(webportal_init());

    ESP_LOGI(TAG, "Local Web Portal Ready");

    if (wifi_setup_process()) 
    { 
        ESP_LOGI(TAG, "Network setup successful"); 
    }
    else 
    { 
        ESP_LOGE(TAG, "Network setup stopped"); 
        return; 
    }

    /*
     *------------------------------------------------------
     * STEP 4: Synchronize time using SNTP
     *
     * IMPORTANT:
     *
     * time_sync_init() already performs the SNTP
     * initialization/synchronization.
     *
     * DO NOT call obtain_time() after this.
     *------------------------------------------------------
     */

    ret = time_sync_init();

    if (ret != ESP_OK) { ESP_LOGE(TAG, "Time unavailable, WebSocket disabled"); return; }

    ESP_LOGI(TAG, "Time Sync - completed");


    /*
     *------------------------------------------------------
     * STEP 5: Verify actual POSIX system time
     *------------------------------------------------------
     */

    ret = verify_system_time();

    if (ret != ESP_OK) { ESP_LOGE(TAG, "System time verification failed"); return; }

    /*
     *------------------------------------------------------
     * STEP 6: Print memory before microphone
     *------------------------------------------------------
     */

    ESP_LOGI(TAG, "Memory status before microphone initialization");

    print_memory_status();


    /*
     *------------------------------------------------------
     * STEP 7: Initialize microphone
     *------------------------------------------------------
     */

    ESP_LOGI(TAG, "Initializing microphone");

    ret = inmp441_init();

    if (ret != ESP_OK) { ESP_LOGE(TAG, "INMP441 initialization failed"); return; }

    ESP_LOGI(TAG, "Microphone ready");


    /*
     *------------------------------------------------------
     * STEP 8: Print memory before WebSocket
     *------------------------------------------------------
     */

    ESP_LOGI(TAG, "Memory status before WebSocket initialization");

    print_memory_status();

    /*
     *------------------------------------------------------
     * STEP 9: Initialize WebSocket
     *------------------------------------------------------
     */

    ret = websocket_client_init( "wss://bring-struck-photograph-kinda.trycloudflare.com");

    if (ret != ESP_OK) { ESP_LOGE(TAG, "WebSocket initialization failed: %s", esp_err_to_name(ret)); return; }

    ESP_LOGI(TAG, "WebSocket client started");

    int cnt = 0 ;

    while (1)
    {
        ret =  websocket_send_text( "ESP32_TEST_PACKET :");

        if (ret == ESP_OK) 
        { 
            ESP_LOGI(TAG, "Packet sent"); 
        } 
        else 
        { 
            ESP_LOGE(TAG, "Send failed"); 
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
