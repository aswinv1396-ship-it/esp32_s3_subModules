#include "system_handler.h"

#include "esp_http_server.h"

static const char HOME_PAGE[] =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>ESP32 Web Portal</title>"
"<meta charset=\"UTF-8\">"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"<style>"
"body{"
"background:#0d1117;"
"color:white;"
"font-family:Arial;"
"text-align:center;"
"margin-top:60px;"
"}"

".card{"
"background:#161b22;"
"padding:30px;"
"border-radius:12px;"
"width:350px;"
"margin:auto;"
"}"

"h1{color:#58a6ff;}"

"</style>"
"</head>"

"<body>"

"<div class=\"card\">"

"<h1>ESP32-S3</h1>"

"<h2>Local Web Portal</h2>"

"<p>Welcome</p>"

"<p>Phase-1 Running Successfully</p>"

"</div>"

"</body>"
"</html>";

esp_err_t system_home_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");

    httpd_resp_send(
        req,
        HOME_PAGE,
        HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}