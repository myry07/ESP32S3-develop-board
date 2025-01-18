#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h" //操作gpio的头文件
#include "driver/ledc.h" //pwm波

/*
控制占空比 控制灯的亮度

ESP IDF中使用PWM 可以用LEDC库
*/

#define LED_GPIO GPIO_NUM_2

#define FULL_EV_BIT BIT0
#define EMPTY_EV_BIT BIT1
static EventGroupHandle_t ledc_event; // 事件总句柄

// ledc 回调函数
// IRAM_ATTR 加了该宏 表示放入内存执行 而不是flash
bool IRAM_ATTR ledc_finish_cb(const ledc_cb_param_t *param, void *user_arg)
{
    BaseType_t taskWoken;
    if (param->duty)
    {                              // 占空比为空
        xEventGroupSetBitsFromISR( // 中断服务程序
            ledc_event,            // 句柄
            FULL_EV_BIT,           // 掩码 0 1
            &taskWoken             // 有无高级任务
        );
    }
    else
    {
        xEventGroupSetBitsFromISR(ledc_event, EMPTY_EV_BIT, &taskWoken);
    }

    return taskWoken;
}

void ledFlash(void *param)
{
    EventBits_t ev;

    while (1)
    {
        ev = xEventGroupWaitBits(ledc_event, FULL_EV_BIT | EMPTY_EV_BIT, pdTRUE, pdFALSE, pdMS_TO_TICKS(5000));
        if (ev & FULL_EV_BIT)
        {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
        }

        if (ev & EMPTY_EV_BIT)
        {
            ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 8191, 2000);
            ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
        }

        ledc_cbs_t cbs =
            {
                .fade_cb = ledc_finish_cb};

        ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);
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

    // 定时器配置
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,    // 低速模式
        .timer_num = LEDC_TIMER_0,            // 选择定时器
        .clk_cfg = LEDC_AUTO_CLK,             // 时钟
        .freq_hz = 5000,                      // 评率
        .duty_resolution = LEDC_TIMER_13_BIT, // 占空比分辨率
    };

    ledc_timer_config(&ledc_timer);

    // 定时器通道
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,      // 通道0
        .timer_sel = LEDC_TIMER_0,      // 关联定时器 与ledc_timer.timer_num一致
        .gpio_num = LED_GPIO,           // GPIO管脚
        .intr_type = LEDC_INTR_DISABLE, // 占空比
    };

    ledc_channel_config(&ledc_channel);

    ledc_fade_func_install(0); // 通过该接口 开启硬件PWM

    // 实现呼吸灯效果
    ledc_set_fade_with_time(
        LEDC_LOW_SPEED_MODE, // 低速模式
        LEDC_CHANNEL_0,      // 通道0
        8191,                // 目标占空比  2*13-1 占空比-1
        2000                 // 执行时长
    );

    // 开启通道
    ledc_fade_start(
        LEDC_LOW_SPEED_MODE,
        LEDC_CHANNEL_0,
        LEDC_FADE_NO_WAIT // 无需等待渐变完成 以免阻塞
    );

    ledc_event = xEventGroupCreate;

    // 回调函数 得知是否渐变完成
    ledc_cbs_t cbs =
        {
            .fade_cb = ledc_finish_cb};

    ledc_cb_register(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, &cbs, NULL);

    xTaskCreate(ledFlash, "led", 1024 * 2, NULL, 0, NULL);
}
