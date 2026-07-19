#include "service_handler.h"

#include "esp_log.h"

static const char *TAG = "SERVICE_HANDLER";

esp_err_t service_page_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Service page requested");

    const char *html =
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<title>Services</title>"
        "</head>"
        "<body>"
        "<h1>Service Manager</h1>"
        "<p>Coming Soon...</p>"
        "</body>"
        "</html>";

    httpd_resp_set_type(req, "text/html");

    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}