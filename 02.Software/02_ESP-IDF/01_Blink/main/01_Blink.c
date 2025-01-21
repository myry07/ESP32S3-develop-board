#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" //操作gpio的头文件
#include "esp_log.h"

#define LED_GPIO GPIO_NUM_2

void ledFlash(void *param)
{
    int gpio_level = 0;

    while (1)
    {
        gpio_level = gpio_level ? 0 : 1;
        gpio_set_level(LED_GPIO, gpio_level);
        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms

        if (gpio_level == 0)
        {
            ESP_LOGI("main", "turn off");
        }
        else
        {
            ESP_LOGI("main", "turn on");
        }
    }
}

void app_main(void)
{
    
    // GPIO配置
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << LED_GPIO),       // 按位掩码 可控制多个gpio
        .pull_up_en = GPIO_PULLUP_DISABLE,     // 使能上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 使能下拉
        .mode = GPIO_MODE_OUTPUT,              // 输出模式
        .intr_type = GPIO_INTR_DISABLE,        // 中断
    };

    gpio_config(&led_cfg);

    xTaskCreate(ledFlash, "led", 1024 * 2, NULL, 0, NULL);
}
