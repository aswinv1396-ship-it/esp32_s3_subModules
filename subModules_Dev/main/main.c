#include "rgb_led.h"
#include "rgb_color.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



void app_main(void)
{

    rgb_led_init();


    while(1)
    {

        rgb_color_set("red",6);

        vTaskDelay(pdMS_TO_TICKS(1000));


        rgb_color_set("orange",70);

        vTaskDelay(pdMS_TO_TICKS(1000));


        rgb_color_set_enum(COLOR_BLUE,10);

        vTaskDelay(pdMS_TO_TICKS(1000));


        rgb_led_set_color(255,20,100,20);

        vTaskDelay(pdMS_TO_TICKS(1000));

    }

}
