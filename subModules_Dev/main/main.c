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
#include "oled.h"

static const char *TAG = "MAIN";

#define MAX_ATTEMPTS 4
#define MIC_TASK_STACK_SIZE    8192
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
/*
 * Console Function: Find Saved Network
 * Updated to check the latest active profile (index 0) in the history array
 */
int find_saved_network(wifi_ap_info_t *aps, int ap_count)
{
    // Check if the latest slot 0 in the history array has valid data
    if (!saved_wifi[0].valid)
    {
        return -1;
    }

    for (int i = 0; i < ap_count; i++)
    {
        // Compare scanned networks against slot 0
        if (strcmp(saved_wifi[0].ssid, aps[i].ssid) == 0)
        {
            return i;
        }
    }

    return -1;
}

/*
 * Console Function: Connect to Saved Network
 * Updated to utilize slot 0 credentials from your history array
 */
bool connect_saved_network(wifi_ap_info_t *ap)
{
    console_print("\nConnecting to saved network: %s\n", ap->ssid);

    // Read credentials strictly from index 0
    esp_err_t ret = wifi_manager_connect(saved_wifi[0].ssid, saved_wifi[0].password);

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

/*
 * Console Function: Interactive User Configuration Process
 * Updated to properly reference index 0 for notifications
 */
static bool wifi_setup_process(void)
{
    wifi_ap_info_t aps[WIFI_MAX_AP];

    while(1)
    {
        console_print("\nScanning WiFi networks...\n");

        int ap_count = wifi_manager_scan(aps, WIFI_MAX_AP);
        if(ap_count <= 0)
        {
            console_print("No WiFi networks found\n");
            return false;
        }

        console_print("\nAvailable WiFi Networks:\n");
        console_print("============================\n");
        for(int i = 0; i < ap_count; i++)
        {
            console_print("%d) SSID: %s  RSSI:%d  CH:%d\n", i + 1, aps[i].ssid, aps[i].rssi, aps[i].channel);
        }
        console_print("============================\n");

        int saved_index = find_saved_network(aps, ap_count);
        if (saved_index >= 0)
        {
            char ans[8];
            
            // Fixed: Referencing slot 0 array syntax cleanly here
            console_print("\nSaved network \"%s\" found.\n", saved_wifi[0].ssid);       
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

        int selection = console_read_int("Select WiFi number: ");
        if(selection < 1 || selection > ap_count)
        {
            console_print("Invalid WiFi selection\n");
            continue;
        }

        int attempt = 0;
        /*
         * Password retry loop
         */
        while(attempt < MAX_ATTEMPTS)
        {
            char password[64];
            console_read_string("Enter password: ", password, sizeof(password));

            console_print("\nConnecting to %s...\n", aps[selection-1].ssid);

            // Prime active credential states before connecting
            wifi_manager_set_selected_ssid(aps[selection-1].ssid);
            wifi_manager_set_password(password);

            esp_err_t ret = wifi_manager_connect(aps[selection-1].ssid, password);

            if(ret == ESP_OK && wifi_manager_is_connected())
            {
                console_print("WiFi connected\n");

                char ip[16];
                if(wifi_manager_get_ip(ip, sizeof(ip)) == ESP_OK)
                {
                    console_print("IP Address : %s\n", ip);
                }
 
                if(check_internet())
                {
                    console_print("Internet available\n");
                    
                    // Saves credentials, moving this profile straight to top of history (index 0)
                    wifi_manager_save_credentials();
                    return true;
                }
                else
                {
                    console_print("WiFi connected but internet unavailable\n");
                }
            }
            else
            {
                console_print("Wrong password or connection failed\n");
            }

            attempt++;
            console_print("Attempt %d/%d failed\n", attempt, MAX_ATTEMPTS);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        char answer[8];
        console_read_string("\nMaximum attempts reached. Scan again? (y/n): ", answer, sizeof(answer));

        if(answer[0] == 'y' || answer[0] == 'Y')
        {
            console_print("Restarting WiFi scan...\n");
            continue;
        }
        else
        {
            console_print("Exiting WiFi configuration\n");
            return false;
        }
    }
}

/* --- Forward declaration for our isolated console worker thread --- */
static void wifi_console_task(void *pvParameters)
{
    ESP_LOGI("CONSOLE_WIFI", "Interactive Serial Console WiFi Worker Thread Started.");
    
    // This can now run its loops and block safely without stopping your web dashboard!
    if (wifi_setup_process()) 
    { 
        ESP_LOGI("CONSOLE_WIFI", "Network setup via serial console successful!"); 
    }
    else 
    { 
        ESP_LOGE("CONSOLE_WIFI", "Network setup via serial console stopped."); 
    }
    
    vTaskDelete(NULL); // Destroy task cleanly when exited
}

void app_main(void)
{
    console_manager_init();

    if (oled_init() == ESP_OK) 
    {
        oled_write_line(0, "SYSTEM BOOTING...");
        oled_write_line(1, "Hotspot active.");
    }

    esp_err_t ret = wifi_manager_init();
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

    // 1. Load the history array of 5 profiles out of NVS flash memory
    wifi_manager_load_credentials();

    // 2. Fire a local network scan to see what routers are currently visible
    ESP_LOGI(TAG, "Starting background scan for known networks...");
        
    wifi_ap_info_t *scanned_networks = malloc(WIFI_MAX_AP * sizeof(wifi_ap_info_t));
    if (scanned_networks == NULL) 
    {
        ESP_LOGE(TAG, "Failed to allocate memory for scan results!");
        return;
    }
    
    int scanned_count = wifi_manager_scan(scanned_networks, WIFI_MAX_AP);

    // 3. Let the matching engine try your 5 saved slots one-by-one
    bool auto_connected = wifi_manager_scan_and_connect_known(scanned_networks, scanned_count);

    free(scanned_networks);

    if (!auto_connected)
    {
        ESP_LOGW(TAG, "⚠️ No known networks found or connection failed.");
        ESP_LOGW(TAG, "📌 Portal Active: Connect to 'ESP32-Assistant' OR use Serial Terminal input.");

        /* 
         *  FIX 1: Spawn the console configuration menu into its own isolated thread.
         * This protects your main loop from stack overflows and keeps it running.
         */
        xTaskCreate(wifi_console_task, "wifi_console_task", 4096, NULL, 3, NULL);

        /* 
         * FIX 2: Dynamic Wait Trap.
         * Pause main boot pipeline until EITHER the Web Portal OR the Serial Console 
         * successfully connects the device to an external router interface.
         */
        while (!wifi_manager_is_connected())
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        ESP_LOGI(TAG, "🎉 Network connection established! Resuming system boot...");
    }
	
    /*
     *------------------------------------------------------
     * STEP 4: Synchronize time using SNTP (Guaranteed to have internet now!)
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

    ret = websocket_client_init("wss://donor-planet-suggesting-longitude.trycloudflare.com");
    if (ret != ESP_OK) 
    { 
        ESP_LOGE(TAG, "WebSocket initialization failed: %s", esp_err_to_name(ret)); 
        return; 
    }
    ESP_LOGI(TAG, "WebSocket client started");

  
    while (!websocket_client_is_connected())
    {
        ESP_LOGI(TAG, "Waiting for WebSocket connection...");
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    xTaskCreate(mic_task, "mic_task", MIC_TASK_STACK_SIZE, NULL, MIC_TASK_PRIORITY, NULL);

    UBaseType_t watermark = uxTaskGetStackHighWaterMark(NULL);
    ESP_LOGI(TAG, "Mic task remaining stack: %u", (unsigned int)watermark);
    */
    while (1)
    {
        ret = websocket_send_text("ESP32_TEST_PACKET :");
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}
