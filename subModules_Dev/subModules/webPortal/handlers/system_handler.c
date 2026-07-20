#include "system_handler.h"
#include "esp_http_server.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/temperature_sensor.h"
#include "esp_log.h"

// Reference external layouts safely
extern const char COMMON_CSS[];
extern const char COMMON_NAV[];
extern const char SYSTEM_CONTENT[];

static const char *TAG = "SYSTEM_HANDLER";

/*-------------------------------------------------------
 * HTML Page Delivery Route
 *------------------------------------------------------*/
esp_err_t system_page_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><head><meta charset='UTF-8'>");
    httpd_resp_sendstr_chunk(req, "<meta name='viewport' content='width=device-width, initial-scale=1'><title>ESP32-S3 System</title><style>");
    httpd_resp_sendstr_chunk(req, COMMON_CSS);
    httpd_resp_sendstr_chunk(req, "</style></head><body>");
    
    httpd_resp_sendstr_chunk(req, SYSTEM_CONTENT);
    httpd_resp_sendstr_chunk(req, COMMON_NAV);
    
    httpd_resp_sendstr_chunk(req, "</body></html>");
    httpd_resp_sendstr_chunk(req, NULL); // Complete transmission transaction

    return ESP_OK;
}

/*-------------------------------------------------------
 * Real-Time Telemetry JSON API Endpoint Handler
 *------------------------------------------------------*/
esp_err_t system_status_api_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "application/json");

    // 1. Calculate operational uptime values
    int64_t total_seconds = esp_timer_get_time() / 1000000;
    int days = total_seconds / 86400;
    int hours = (total_seconds % 86400) / 3600;
    int mins = (total_seconds % 3600) / 60;
    int secs = total_seconds % 60;

    char uptime_str[64];
    if (days > 0) {
        snprintf(uptime_str, sizeof(uptime_str), "%dd %dh %dm %ds", days, hours, mins, secs);
    } else {
        snprintf(uptime_str, sizeof(uptime_str), "%dh %dm %ds", hours, mins, secs);
    }

    // 2. Fetch memory structures directly from internal trackers
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_heap = esp_get_minimum_free_heap_size();

    // 3. Acquire on-chip diagnostic temperatures 
    float tsens_out = 0.0;
    static temperature_sensor_handle_t temp_sensor = NULL;
    
    // Lazy initialisation wrapper for the integrated sensor module
    if (temp_sensor == NULL) {
        temperature_sensor_config_t temp_cfg = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
        if (temperature_sensor_install(&temp_cfg, &temp_sensor) == ESP_OK) 
        {
            temperature_sensor_enable(temp_sensor);
        }
    }
    if (temp_sensor != NULL) 
    {
        temperature_sensor_get_celsius(temp_sensor, &tsens_out);
    }

    // 4. Estimate baseline power drain based on active peripherals (STA+AP mode active = ~140-240mA)
    uint32_t est_current = 160; 
    if (free_heap < 120000) est_current += 35; // Increment estimate if execution stack footprint rises

    // 5. Build and pack JSON response payload string
    char json_response[256];
    snprintf(json_response, sizeof(json_response),
             "{"
             "\"uptime\":\"%s\","
             "\"free_heap\":%lu,"
             "\"min_heap\":%lu,"
             "\"temperature\":%.2f,"
             "\"est_current\":%lu"
             "}",
             uptime_str, (unsigned long)free_heap, (unsigned long)min_heap, tsens_out, (unsigned long)est_current);

    return httpd_resp_sendstr(req, json_response);
}


