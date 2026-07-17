#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_netif.h"

#include "wifi_manager.h"


static const char *TAG = "MAIN";


/*
 * Check internet connectivity
 */
/*
 * Check internet connectivity using DNS + HTTP request
 */
static bool check_internet(void)
{
    struct addrinfo hints = {0};
    struct addrinfo *res = NULL;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;


    ESP_LOGI(TAG, "Resolving google.com");


    int err = getaddrinfo("google.com",
                          "80",
                          &hints,
                          &res);


    if(err != 0 || res == NULL)
    {
        ESP_LOGE(TAG,
                 "DNS failed: %d",
                 err);

        return false;
    }



    int sock = socket(res->ai_family,
                      res->ai_socktype,
                      res->ai_protocol);



    if(sock < 0)
    {
        ESP_LOGE(TAG,
                 "Socket creation failed");

        freeaddrinfo(res);

        return false;
    }



    struct timeval timeout =
    {
        .tv_sec = 5,
        .tv_usec = 0
    };


    /*
     * Timeout for connect()
     */
    setsockopt(sock,
               SOL_SOCKET,
               SO_SNDTIMEO,
               &timeout,
               sizeof(timeout));


    /*
     * Timeout for recv()
     */
    setsockopt(sock,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &timeout,
               sizeof(timeout));



    ESP_LOGI(TAG,
             "Connecting to google.com");


    int ret =
        connect(sock,
                res->ai_addr,
                res->ai_addrlen);



    if(ret != 0)
    {
        ESP_LOGE(TAG,
                 "Connection failed");

        close(sock);
        freeaddrinfo(res);

        return false;
    }



    freeaddrinfo(res);



    /*
     * Send HTTP request
     */
    const char *request =
        "GET / HTTP/1.1\r\n"
        "Host: google.com\r\n"
        "Connection: close\r\n"
        "\r\n";



    send(sock,
         request,
         strlen(request),
         0);



    char buffer[128];


    int len =
        recv(sock,
             buffer,
             sizeof(buffer)-1,
             0);



    close(sock);



    if(len <= 0)
    {
        ESP_LOGE(TAG,
                 "No HTTP response");

        return false;
    }


    buffer[len] = '\0';



    ESP_LOGI(TAG,
             "HTTP response: %.40s",
             buffer);



    /*
     * Check HTTP status
     */
     if(strstr(buffer, "HTTP/1.1") || strstr(buffer, "HTTP/1.0"))
	{
    	return true;
	}

    return false;
}



void app_main(void)
{

    /*
     * Initialize WiFi
     */
    esp_err_t ret =
        wifi_manager_init();


    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "WiFi init failed");

        return;
    }



    /*
     * Connect to selected network
     */
    const char *ssid =
            "test";


    const char *password =
            "1234567v";



    ESP_LOGI(TAG,
             "Connecting to %s",
             ssid);



    ret =
        wifi_manager_connect(
                ssid,
                password);



    if(ret != ESP_OK)
    {
        ESP_LOGE(TAG,
                 "WiFi connection failed");

        return;
    }



    ESP_LOGI(TAG,
             "WiFi connected");



    /*
     * Get IP address
     */
    char ip[16];


    if(wifi_manager_get_ip(ip,sizeof(ip))
            == ESP_OK)
    {
        ESP_LOGI(TAG,
                 "IP Address: %s",
                 ip);
    }



    /*
     * Check internet
     */
    if(check_internet())
    {
        ESP_LOGI(TAG,
                 "Internet available");
    }
    else
    {
        ESP_LOGE(TAG,
                 "No internet access");
    }



    while(1)
    {
        vTaskDelay(
            pdMS_TO_TICKS(1000));
    }

}
