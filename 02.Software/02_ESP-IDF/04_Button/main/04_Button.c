#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "iot_button.h"
#include "driver/gpio.h" //操作gpio的头文件

#define BOOT_BUTTON_NUM 0

static const char *TAG = "button_power_save";

static void button_event_cb(void *arg, void *data)
{
    iot_button_print_event((button_handle_t)arg);
}

void button_init(uint32_t button_num)
{
    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1 << button_num),    // 按位掩码 可控制多个gpio
        .pull_up_en = GPIO_PULLUP_DISABLE,    // 使能上拉
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // 使能下拉
        .mode = GPIO_MODE_INPUT,              
        .intr_type = GPIO_INTR_DISABLE,       // 中断
    };

    gpio_config(&gpio_cfg);

    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num = button_num,
            .active_level = 0,
        },
    };
    button_handle_t btn = iot_button_create(&btn_cfg);
    assert(btn);
    esp_err_t err = iot_button_register_cb(btn, BUTTON_PRESS_DOWN, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_UP, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_REPEAT_DONE, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_SINGLE_CLICK, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_DOUBLE_CLICK, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_START, button_event_cb, NULL);
    // err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_HOLD, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_LONG_PRESS_UP, button_event_cb, NULL);
    err |= iot_button_register_cb(btn, BUTTON_PRESS_END, button_event_cb, NULL);

    ESP_ERROR_CHECK(err);
}

void app_main(void)
{
    button_init(BOOT_BUTTON_NUM);
}
