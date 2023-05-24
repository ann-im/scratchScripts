#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define BLINK_GPIO 32
#define TOGGLE_HALFRATE 500
#define ONE_MINUTE 60000

void app_main(void)
{
    gpio_pad_select_gpio(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int iterations = 0;
    TickType_t last_wake_time = xTaskGetTickCount();

    while(1) {
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(TOGGLE_HALFRATE / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(TOGGLE_HALFRATE / portTICK_PERIOD_MS);

        iterations++;

        if (xTaskGetTickCount() - last_wake_time >= ONE_MINUTE / portTICK_PERIOD_MS) {
            printf("Iterations completed: %d\n", iterations);
            last_wake_time = xTaskGetTickCount();
        }
    }
}
