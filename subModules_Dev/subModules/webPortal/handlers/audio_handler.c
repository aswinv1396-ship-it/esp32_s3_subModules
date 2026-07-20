#include "audio_handler.h"

#include "esp_http_server.h"

#include "html/audio_html.h"
#include "html/common_css.h"
#include "html/common_nav.h"



esp_err_t audio_page_handler(httpd_req_t *req)
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
        "<title>ESP32-S3 Audio</title>"
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
     * Audio page content
     */
    httpd_resp_sendstr_chunk(
        req,
        AUDIO_CONTENT
    );


    /*
     * Common navigation bar
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
     * End chunk response
     */
    httpd_resp_sendstr_chunk(
        req,
        NULL
    );


    return ESP_OK;
}
