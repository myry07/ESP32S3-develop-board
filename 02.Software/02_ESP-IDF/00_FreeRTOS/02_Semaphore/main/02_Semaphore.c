#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht11.h"

SemaphoreHandle_t bin_sem; // 创建二进制信号

/* 释放任务 */
void giveTask(void *param)
{
    while (1)
    {
        xSemaphoreGive(bin_sem);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* 获取任务 */
void takeTask(void *param)
{
    while (1)
    {
        if (pdTRUE == xSemaphoreTake(bin_sem, portMAX_DELAY)) // portMAX_DELAY 无限等待
        {
            ESP_LOGI("take: ", "successful");
        }
        else
        {
            ESP_LOGI("take: ", "failed");
        }
    }
}

void app_main(void)
{
    /* 创建句柄 */
    bin_sem = xSemaphoreCreateBinary();

    /* 创建任务 */
    xTaskCreatePinnedToCore(giveTask, "give", 1024 * 2, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(takeTask, "take", 1024 * 2, NULL, 2, NULL, 1);
}

