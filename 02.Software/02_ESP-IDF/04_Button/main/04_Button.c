#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define Button_GPIO GPIO_NUM_0
#define TAG "app"

void gpioInit()
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << Button_GPIO), // 按位掩码 可控制多个gpio
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .mode = GPIO_MODE_INPUT,        // 输入模式
        .intr_type = GPIO_INTR_DISABLE, // 中断
    };

    if (gpio_config(&led_cfg) != NULL)
    {
        ESP_LOGI(TAG, "GPIO Init");
    };
}

void buttonTask(void *param)
{
    gpioInit();

    while (1)
    {
        if (0 == gpio_get_level(Button_GPIO))
        {

            /* 软件去抖 */
            while (0 == gpio_get_level(Button_GPIO))
            {
                vTaskDelay(pdMS_TO_TICKS(20));
            }

            ESP_LOGI(TAG, "Key pressed");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_main(void)
{
    xTaskCreate(buttonTask, "button", NULL, 1024 * 2, 0, NULL);
}
