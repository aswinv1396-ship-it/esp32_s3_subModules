#include "system_handler.h"

#include "esp_http_server.h"

#include "html/system_html.h"
#include "html/common_css.h"
#include "html/common_nav.h"



esp_err_t system_page_handler(httpd_req_t *req)
{

    httpd_resp_set_type(req, "text/html");


    /*
     * HTML Header + CSS
     */
    httpd_resp_sendstr_chunk(
        req,
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>ESP32-S3 System</title>"
        "<style>"
    );


    /*
     * Common CSS
     */
    httpd_resp_sendstr_chunk(
        req,
        COMMON_CSS
    );


    /*
     * Close CSS and open body
     */
    httpd_resp_sendstr_chunk(
        req,
        "</style>"
        "</head>"
        "<body>"
    );


    /*
     * System page content
     */
    httpd_resp_sendstr_chunk(
        req,
        SYSTEM_CONTENT
    );


    /*
     * Common bottom navigation
     */
    httpd_resp_sendstr_chunk(
        req,
        COMMON_NAV
    );


    /*
     * HTML Footer
     */
    httpd_resp_sendstr_chunk(
        req,
        "</body>"
        "</html>"
    );


    /*
     * End response
     */
    httpd_resp_sendstr_chunk(
        req,
        NULL
    );


    return ESP_OK;
}
