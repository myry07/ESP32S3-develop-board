#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "WS";

#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 1;
static uint8_t rgb = -1;
static uint32_t value = 30;

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (s_led_state)
    {
        /* turn on */

        switch (rgb)
        {
        case 0: // r
            led_strip_set_pixel(led_strip, 0, value, 0, 0);
            ESP_LOGI(TAG, "Turning the LED %s!", "ON Red");
            break;
        case 1: // g
            led_strip_set_pixel(led_strip, 0, 0, value, 0);
            ESP_LOGI(TAG, "Turning the LED %s!", "ON Green");
            break;
        case 2: // b
            led_strip_set_pixel(led_strip, 0, 0, 0, value);
            ESP_LOGI(TAG, "Turning the LED %s!", "ON Blue");
            break;
        }
        led_strip_refresh(led_strip);
    }
    else
    {
        /* turn off */
        led_strip_clear(led_strip);
        ESP_LOGI(TAG, "Turning the LED %s!", "OFF");
    }
}

static void ledInit(void)
{
    ESP_LOGI(TAG, "LED init!");
    /* init */
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}


void app_main(void)
{
    ledInit();

    while (1)
    {
        rgb++;
        blink_led();
        if (rgb == 2)
        {
            rgb = -1;
        }
        /* Toggle the LED state */
        // s_led_state = !s_led_state;
        // vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
