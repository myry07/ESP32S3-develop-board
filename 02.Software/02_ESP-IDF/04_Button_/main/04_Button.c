#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" //操作gpio的头文件

#define Button_GPIO GPIO_NUM_0

void gpioInit()
{
    gpio_config_t led_cfg = {
        .pin_bit_mask = (1 << Button_GPIO),    // 按位掩码 可控制多个gpio
        .pull_up_en = GPIO_PULLUP_ENABLE,      // 使能上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 使能下拉
        .mode = GPIO_MODE_INPUT,               // 输入模式
        .intr_type = GPIO_INTR_DISABLE,        // 中断
    };

    gpio_config(&led_cfg);
}

void buttonTask(void *param)
{
    gpioInit();

    // 初始化变量来存储按钮的当前和上一个状态
    bool button_state = 0;
    bool last_button_state = 0;

    while (1)
    {
        // 读取当前按钮状态
        button_state = gpio_get_level(Button_GPIO);

        // 检查按钮状态是否发生变化
        if (button_state != last_button_state)
        {
            if (button_state == 0)
            {
                printf("Button pressed\n");
            }
            else
            {
                printf("Button released\n");
            }
            last_button_state = button_state;
        }

        // 延时一段时间以避免过于频繁的读取
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    xTaskCreate(buttonTask, "button", NULL, 1024 * 2, 0, NUll);
}
