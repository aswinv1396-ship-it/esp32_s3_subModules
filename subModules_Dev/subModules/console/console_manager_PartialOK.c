#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdarg.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "console_manager.h"


#define TAG "CONSOLE_MANAGER"



void console_manager_init(void)
{
    ESP_LOGI(TAG,"USB Console Ready");

    printf("\n===============================\n");
    printf(" ESP32 USER INPUT TEST\n");
    printf("===============================\n");

    fflush(stdout);
}



/*
 * Wait until USB console has data
 */
static bool console_wait_input(uint32_t timeout_ms)
{

    fd_set set;

    struct timeval timeout;


    FD_ZERO(&set);

    FD_SET(STDIN_FILENO,&set);


    timeout.tv_sec =
        timeout_ms / 1000;


    timeout.tv_usec =
        (timeout_ms % 1000) * 1000;



    int ret =
        select(
            STDIN_FILENO + 1,
            &set,
            NULL,
            NULL,
            &timeout
        );


    return (ret > 0);

}



/*
 * Remove remaining characters
 */
static void console_flush_input(void)
{

    int c;


    while((c=getchar()) != '\n' &&
          c != EOF)
    {

    }


}



/*
 * Read line
 */
 
bool console_read_line(const char *prompt,
                       char *buffer,
                       size_t length)
{
    if(buffer == NULL || length == 0)
    {
        return false;
    }


    printf("%s", prompt);
    fflush(stdout);


    while(1)
    {

        if(fgets(buffer, length, stdin) != NULL)
        {

            buffer[strcspn(buffer, "\r\n")] = '\0';

            return true;
        }


        /*
         * No input yet
         */
        clearerr(stdin);


        /*
         * Wait silently
         */
        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );

    }

}


/*
 * Integer input
 */
int console_read_int(const char *prompt)
{
    char buffer[32];


    if(!console_read_line(prompt,
                          buffer,
                          sizeof(buffer)))
    {
        return -1;
    }


    return atoi(buffer);
}



/*
 * String input
 */
bool console_read_string(
        const char *prompt,
        char *buffer,
        size_t length)
{

    return console_read_line(
            prompt,
            buffer,
            length
        );

}



/*
 * Print helper
 */
void console_print(
        const char *fmt,
        ...)
{

    va_list args;


    va_start(args,fmt);


    vprintf(fmt,args);


    va_end(args);


    fflush(stdout);

}
