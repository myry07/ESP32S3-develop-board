#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "imgs.h"

#define BUTTON_PIN 0

TFT_eSPI tft = TFT_eSPI();

byte state = 0;
bool needRefresh = true; // 标志位，控制是否需要刷新屏幕

void showTask(void *param)
{
  while (1)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(50); // 消抖
      if (digitalRead(BUTTON_PIN) == LOW)
      {
        while (digitalRead(BUTTON_PIN) == LOW); // 等待按钮释放
        if (state < 2)
        {
          state++;
        }
        else
        {
          state = 0;
        }
        needRefresh = true; // 设置需要刷新屏幕
        tft.fillScreen(TFT_BLACK); // 清屏
        Serial.printf("state: %d\n", state);
      }
    }

    if (needRefresh) // 只在需要时刷新屏幕
    {
      switch (state)
      {
      case 0:
        tft.pushImage(20, 0, Z_WIDTH, Z_HEIGHT, z);
        break;

      case 1:
        tft.pushImage(40, 0, P2_WIDTH, P2_HEIGHT, p2);
        break;

      case 2:
        tft.pushImage(40, 0, P3_WIDTH, P3_HEIGHT, p3);
        break;

      default:
        break;
      }
      needRefresh = false; // 重置标志位
    }

    vTaskDelay(50); // 增加延迟，减少刷新频率
  }
}

void displayInit()
{
  tft.begin();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  Serial.println("Display init");
}

void setup()
{
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // 启用上拉电阻
  displayInit();
  xTaskCreate(showTask, "show", 1024 * 2, NULL, 1, NULL);
}

void loop()
{
  // 主循环为空，任务由 FreeRTOS 调度
}