#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void helloTask(void *param)
{
    while (1)
    {
        ESP_LOGI("task: ", "Hello World!");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    xTaskCreatePinnedToCore(helloTask, "hello", 2048, NULL, 1, NULL, 1);
}

