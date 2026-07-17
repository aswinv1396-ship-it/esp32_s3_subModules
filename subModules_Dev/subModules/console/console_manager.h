#ifndef CONSOLE_MANAGER_H
#define CONSOLE_MANAGER_H


#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"


void console_manager_init(void);


void console_print(
        const char *format,
        ...
);


int console_read_int(
        const char *prompt
);


bool console_read_string(
        const char *prompt,
        char *buffer,
        size_t size
);


bool console_read_yes_no(
        const char *prompt
);


#endif
