#include "rgb_led.h"
#include "rgb_color.h"

#include "button.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

#include "wifi_manager.h"


void app_main(void)
{
	/*
	button_init();
	
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
    */
    
    /*
    bool previous_state = false;


    while(1)
    {

        bool current_state = button_is_pressed();

        if(current_state && !previous_state)
        {
            printf("BUTTON PRESSED\n");
        }

        
        if(current_state)
        {
            printf("BUTTON HELD\n");
        }

        
        if(!current_state && previous_state)
        {
            printf("BUTTON RELEASED\n");
        }

        previous_state = current_state;
        
        vTaskDelay(pdMS_TO_TICKS(50));
	}

	*/

    printf("Starting WiFi test\n");


    wifi_manager_init();


    printf("WiFi init OK\n");


  
    

}
