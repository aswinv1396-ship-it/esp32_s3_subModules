#include "time_sync.h"

#include "esp_log.h"
#include "esp_sntp.h"

#include "time.h"
#include "sys/time.h"


static const char *TAG = "SNTP";


static void print_current_time(void)
{
    time_t now;
    struct tm timeinfo;

    time(&now);

    localtime_r( &now, &timeinfo);

    ESP_LOGI(TAG,"Current time: %04d-%02d-%02d %02d:%02d:%02d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour,
        timeinfo.tm_min, timeinfo.tm_sec);
}

esp_err_t time_sync_init(void)
{
    ESP_LOGI(TAG, "Starting SNTP");


    /*
     * Set Indian timezone
     * Change if required
     */
    setenv(
        "TZ",
        "IST-5:30",
        1
    );

    tzset();



    /*
     * Stop previous SNTP instance
     */
    esp_sntp_stop();



    /*
     * Configure SNTP
     */
    esp_sntp_setoperatingmode(
        ESP_SNTP_OPMODE_POLL
    );


    /*
     * Multiple NTP fallback servers
     */
    esp_sntp_setservername(
        0,
        "time.google.com"
    );


    esp_sntp_setservername(
        1,
        "pool.ntp.org"
    );


    esp_sntp_setservername(
        2,
        "time.cloudflare.com"
    );


    esp_sntp_init();



    time_t now = 0;

    struct tm timeinfo = {0};



    int retry = 0;


    while(1)
    {

        time(&now);


        localtime_r(
            &now,
            &timeinfo
        );


        /*
         * Year should be > 2024
         */
        if(timeinfo.tm_year >= (2024 - 1900))
        {
            break;
        }



        ESP_LOGI(TAG, "Waiting for time sync... (%d)", retry);


        /*
         * Longer wait after retries
         */
        if(retry < 10)
        {
            vTaskDelay(
                pdMS_TO_TICKS(2000)
            );
        }
        else
        {
            vTaskDelay(
                pdMS_TO_TICKS(5000)
            );
        }


        retry++;



        if(retry > 30)
        {
            ESP_LOGE(TAG, "SNTP failed");


            /*
             * Print whatever time exists
             */
            print_current_time();


            return ESP_FAIL;
        }
    }



    ESP_LOGI(TAG, "Time synchronized successfully");


    print_current_time();



    return ESP_OK;
}
