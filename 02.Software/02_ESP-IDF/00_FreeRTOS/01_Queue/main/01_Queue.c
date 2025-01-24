#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>

QueueHandle_t queue_handle = NULL; // 队列指针 句柄

// 结构体 队列数据单元
typedef struct
{
    int value;
} queue_data;

void receiveTask(void *param)
{
    queue_data qd;
    // 队列接收的返回类型为 BaseType_t
    while (1)
    {
        if (pdTRUE == xQueueReceive(queue_handle, &qd, 100))
        {
            ESP_LOGI("receiveTask:", "value: %d", qd.value);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void sendTask(void *param)
{
    queue_data qd;
    memset(&qd, 0, sizeof(queue_data)); // 初始化

    while (1)
    {
        if (pdTRUE == xQueueSend(queue_handle, &qd, 100))
        {
            ESP_LOGI("sendTask", "sent");
        }
        else
        {
            ESP_LOGI("sendTask", "failed");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
        qd.value++;
    }
}
void app_main(void)
{
    /* 创建队列 */
    queue_handle = xQueueCreate(10, sizeof(queue_data));

    xTaskCreatePinnedToCore(sendTask, "send", 1024 * 2, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(receiveTask, "receive", 1024 * 2, NULL, 1, NULL, 1);
}
