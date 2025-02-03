#include <TFT_eSPI.h>
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "ui.h"
#include "FreeRTOS.h"
#include "freertos/task.h"
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48  // 连接WS2812B的引脚，这里是D48
#define LED_COUNT 1 // WS2812B灯珠的数量，按需修改

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
uint16_t calData[5] = {189, 3416, 359, 3439, 1};

TFT_eSPI tft = TFT_eSPI(); /* TFT entity */

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 13];

byte led_state = 0;

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();

  int16_t x1 = screenWidth - area->x2 - 1;
  int16_t y1 = screenHeight - area->y2 - 1;

  tft.setAddrWindow(x1, y1, w, h);

  for (uint32_t i = 0; i < w * h; i++)
  {
    uint16_t color = color_p[w * h - 1 - i].full; // 倒序绘制
    tft.pushColor(color);
  }

  tft.endWrite();

  lv_disp_flush_ready(disp);
}

uint16_t touchX, touchY;
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  bool touched = tft.getTouch(&touchX, &touchY, 600);
  if (!touched)
  {
    data->state = LV_INDEV_STATE_REL;
  }
  else
  {
    data->state = LV_INDEV_STATE_PR;

    data->point.x = screenWidth - touchX - 1;
    data->point.y = screenHeight - touchY - 1;

    Serial.print("Data x: ");
    Serial.println(data->point.x);
    Serial.print("Data y: ");
    Serial.println(data->point.y);
  }
}

void lvglInit()
{
  // lvgl init
  lv_init();

  lv_disp_draw_buf_init(&draw_buf, buf1, NULL, screenWidth * screenHeight / 13);

  /* Initialize the display */
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  /* Change the following line to your display resolution */
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  // 旋转
  //  disp_drv.sw_rotate = 1;  // If you turn on software rotation, Do not update or replace LVGL
  //  disp_drv.rotated = LV_DISP_ROT_180;
  //  disp_drv.full_refresh = 1;  // full_refresh must be 1

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  tft.fillScreen(TFT_BLACK);

  // lv_demo_widgets();  // LVGL demo
  ui_init();
}

void wsInit()
{
  strip.begin();
  strip.show(); // 初始化时先关闭所有灯珠
}


void displayInit()
{

  // LCD init
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTouch(calData);
}

void wsTask(void *param)
{
}
void lvglTask(void *param)
{
  while (1)
  {
    lv_timer_handler();
    lv_tick_inc(5); // 放置5ms过去
    vTaskDelay(5);

    // 创建灯珠
    if (led_state == 1)
    {
      strip.setPixelColor(0, strip.Color(80, 40, 20));
      strip.show();
    }

    // 删除灯珠
    if (led_state == 0)
    {
      strip.setPixelColor(0, strip.Color(0, 0, 0));
      strip.show();
      
    }
  }
}

void setup()
{
  Serial.begin(115200); /*serial init */
  wsInit();
  delay(100);

  displayInit();
  delay(100);

  lvglInit();
  delay(100);

  Serial.println("Setup done");

  xTaskCreate(lvglTask, "lvgl", 1024 * 8, NULL, 3, NULL);
}

void loop()
{
}