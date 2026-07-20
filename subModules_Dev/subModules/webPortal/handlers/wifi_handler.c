#include "wifi_handler.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_wifi.h"

#include "wifi_manager.h"


static const char *TAG = "WIFI_WEB";


#define MAX_SCAN_RESULT 7


/*
 * ----------------------------------------------------
 * GET /api/wifi/scan
 * ----------------------------------------------------
 */

esp_err_t wifi_scan_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG,"WiFi scan requested");

    wifi_ap_info_t aps[WIFI_MAX_AP];

    int count = wifi_manager_scan(  aps,  WIFI_MAX_AP );

    if(count <= 0)
    {
        httpd_resp_set_type(req,"application/json");
        httpd_resp_sendstr(req,
            "{\"networks\":[]}");
        return ESP_OK;
    }

    /*
     * strongest RSSI first
     */
    for(int i=0;i<count-1;i++)
    {
        for(int j=i+1;j<count;j++)
        {
            if(aps[j].rssi > aps[i].rssi)
            {
                wifi_ap_info_t tmp = aps[i];
                aps[i]=aps[j];
                aps[j]=tmp;
            }
        }
    }

    char response[1024];

    int offset=0;

    offset += snprintf( response+offset, sizeof(response)-offset, "{\"networks\":[");

    int limit = count > MAX_SCAN_RESULT ? MAX_SCAN_RESULT : count;

    for(int i=0;i<limit;i++)
    {
        offset += snprintf( response+offset, sizeof(response)-offset, "{\"ssid\":\"%s\",\"rssi\":%d}%s",
            aps[i].ssid,    aps[i].rssi, (i == limit-1) ? "" : ","   );
    }


    snprintf( response+offset,sizeof(response)-offset, "]}" );

    httpd_resp_set_type(req,"application/json" );

    httpd_resp_send(req,  response,  HTTPD_RESP_USE_STRLEN);

	ESP_LOGI(TAG,"WiFi scan completed");

    return ESP_OK;
}


/*-------------------------------------------------------
 * Helper Function: URL Decoding
 * Converts web tokens like %23 back into raw characters like #
 *------------------------------------------------------*/
static void url_decode(char *dst, const char *src) 
{
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit((int)a) && isxdigit((int)b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/*
 * ----------------------------------------------------
 * GET /api/wifi/select?ssid=name
 * ----------------------------------------------------
 */
esp_err_t wifi_select_handler(httpd_req_t *req)
{
    char query[128];

    if(httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID missing");
        return ESP_FAIL;
    }

    char raw_ssid[64] = {0};
    char decoded_ssid[64] = {0};

    if(httpd_query_key_value(query, "ssid", raw_ssid, sizeof(raw_ssid)) != ESP_OK)
    {
        return ESP_FAIL;
    }

    /* 👉 DECODE SSID HERE (Fixes spaces and symbols in network names) */
    url_decode(decoded_ssid, raw_ssid);

    ESP_LOGI(TAG, "Selected SSID=%s", decoded_ssid);

    wifi_manager_set_selected_ssid(decoded_ssid);

    httpd_resp_sendstr(req, "{\"status\":\"selected\"}");

    return ESP_OK;
}

/*
 * ----------------------------------------------------
 * POST /api/wifi/connect
 *
 * body: password=my_password
 * ----------------------------------------------------
 */
esp_err_t wifi_connect_handler(httpd_req_t *req)
{
    char body[128];

    int len = httpd_req_recv(req, body, sizeof(body) - 1);

    if(len <= 0)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "No password");
        return ESP_FAIL;
    }

    body[len] = 0;

    char raw_password[64] = {0};
    char decoded_password[64] = {0};

    if(httpd_query_key_value(body, "password", raw_password, sizeof(raw_password)) != ESP_OK)
    {
        httpd_resp_sendstr(req, "{\"status\":\"missing_password\"}");
        return ESP_OK;
    }

    /* 👉 DECODE PASSWORD HERE (Converts %23%23 back into ##) */
    url_decode(decoded_password, raw_password);

    char *ssid = wifi_manager_get_selected_ssid();

    if(ssid == NULL || strlen(ssid) == 0)
    {
        httpd_resp_sendstr(req, "{\"status\":\"no_ssid_selected\"}");
        return ESP_OK;
    }
    
    // Now logs the clean, true characters
    ESP_LOGI(TAG, "SSID=[%s] PASSWORD=[%s]", wifi_manager_get_selected_ssid(), decoded_password);
    ESP_LOGI(TAG, "Connecting SSID=%s", ssid);    

    /* Pass the clean, decoded password into your connection loop */
    esp_err_t ret = wifi_manager_connect(wifi_manager_get_selected_ssid(), decoded_password);

    if(ret == ESP_OK && wifi_manager_is_connected())
    {
        /* Save the clean decoded password to NVS/credentials storage */
        wifi_manager_set_password(decoded_password);
        wifi_manager_save_credentials();

        httpd_resp_sendstr(req, "{\"status\":\"connected\"}");
        return ESP_OK;
    }

    httpd_resp_sendstr(req, "{\"status\":\"failed\",\"reason\":\"wrong_password\"}");

    return ESP_OK;
}
/*
 * ----------------------------------------------------
 * GET /api/wifi/status
 * ----------------------------------------------------
 */

esp_err_t wifi_status_handler(httpd_req_t *req)
{
    char ip[16] = "0.0.0.0";

    bool connected =
        wifi_manager_is_connected();

    const char *ssid =
        wifi_manager_get_connected_ssid();

    if (ssid == NULL || strlen(ssid) == 0)
    {
        ssid = "Not Connected";
    }

    if (connected)
    {
        wifi_manager_get_ip(ip, sizeof(ip));
    }

    char response[256];

    snprintf(
        response,
        sizeof(response),

        "{"
        "\"connected\":%s,"
        "\"ssid\":\"%s\","
        "\"ip\":\"%s\","
        "\"rssi\":%d"
        "}",

        connected ? "true" : "false",
        ssid,
        ip,
        0
    );

    ESP_LOGI(
        TAG,
        "Status: connected=%s ssid=%s ip=%s",

        connected ? "true" : "false",
        ssid,
        ip
    );

    httpd_resp_set_type(
        req,
        "application/json"
    );

    httpd_resp_send(
        req,
        response,
        HTTPD_RESP_USE_STRLEN
    );

    return ESP_OK;
}
/*
 * ----------------------------------------------------
 * REGISTER ALL WIFI APIs
 * ----------------------------------------------------
 */

void wifi_handler_register(httpd_handle_t server)
{

    httpd_uri_t scan =
    {
        .uri="/api/wifi/scan",
        .method=HTTP_GET,
        .handler=wifi_scan_handler
    };


    httpd_uri_t select =
    {
        .uri="/api/wifi/select",
        .method=HTTP_GET,
        .handler=wifi_select_handler
    };


    httpd_uri_t connect =
    {
        .uri="/api/wifi/connect",
        .method=HTTP_POST,
        .handler=wifi_connect_handler
    };


    httpd_uri_t status =
    {
        .uri="/api/wifi/status",
        .method=HTTP_GET,
        .handler=wifi_status_handler
    };



    httpd_register_uri_handler(server,&scan);

    httpd_register_uri_handler(server,&select);

    httpd_register_uri_handler(server,&connect);

    httpd_register_uri_handler(server,&status);



    ESP_LOGI(TAG,
             "WiFi web APIs registered");
}
