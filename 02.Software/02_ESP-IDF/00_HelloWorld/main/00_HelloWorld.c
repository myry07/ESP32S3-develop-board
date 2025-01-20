#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void printTask(void *param)
{
    while (1)
    {
        ESP_LOGI("main", "Hello World!");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    xTaskCreate(printTask, "print", NULL, 1024, 0, NULL);
}
