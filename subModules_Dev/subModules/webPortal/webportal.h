#ifndef WEBPORTAL_H
#define WEBPORTAL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"
#include "esp_http_server.h"

/*-----------------------------------------------------------
 * Public API
 *----------------------------------------------------------*/

/**
 * @brief Initialize and start HTTP server
 */
esp_err_t webportal_init(void);

/**
 * @brief Stop HTTP server
 */
esp_err_t webportal_stop(void);

/**
 * @brief Get HTTP server handle
 */
httpd_handle_t webportal_get_handle(void);

#ifdef __cplusplus
}
#endif

#endif