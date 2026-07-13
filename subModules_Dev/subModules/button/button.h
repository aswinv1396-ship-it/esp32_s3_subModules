#ifndef BUTTON_H
#define BUTTON_H

#include "driver/gpio.h"
#include <stdbool.h>


#define BUTTON_GPIO GPIO_NUM_4


void button_init(void);

bool button_is_pressed(void);


#endif
