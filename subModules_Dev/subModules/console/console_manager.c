#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "console_manager.h"

#define TAG "CONSOLE_MANAGER"

void console_manager_init(void)
{
    ESP_LOGI(TAG,"USB Console Ready");

    printf("\n===============================\n");
    printf(" ESP32 USER INPUT TEST\n");
    printf("===============================\n\n");

    fflush(stdout);
}

bool console_read_line(const char *prompt, char *buffer, size_t length)
{
    if(buffer == NULL || length == 0)
    {
        return false;
    }
    printf("%s", prompt);
    fflush(stdout);
    size_t index = 0;

    while(1)
    {
        int c = getchar();
        if(c == EOF)
        {
            clearerr(stdin);
            vTaskDelay( pdMS_TO_TICKS(20));
            continue;
        }
        /*
         * ENTER pressed
         */
        if(c == '\n' || c == '\r')
        {
            buffer[index] = '\0';
            printf("\n");
            return true;
        }
        /*
         * Store character
         */
        if(index < length - 1)
        {
            buffer[index++] = (char)c;
        }
        /*
         * Optional echo
         */
        putchar(c);
        fflush(stdout);
    }
}

int console_read_int(const char *prompt)
{

    char buffer[32];
    while(1)
    {
        if(console_read_line(prompt,  buffer,  sizeof(buffer)))
        {
            char *end;
            long value = strtol(buffer,&end,10);
            if(end != buffer && *end == '\0')
            {
                return (int)value;
            }
            printf("Invalid number\n");
        }
    }
}

bool console_read_string(const char *prompt, char *buffer,  size_t length)
{
    return console_read_line(prompt,  buffer, length);
}

void console_print(const char *fmt,...)
{
    va_list args;
    va_start(args,fmt);
    vprintf(fmt,args);
    va_end(args);
    fflush(stdout);
}
