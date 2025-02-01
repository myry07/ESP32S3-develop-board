#include <TFT_eSPI.h>
#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "ui.h"
// #include <demos/lv_demos.h>
// #include <examples/lv_examples.h>

// #define CrowPanel_24
#define CrowPanel_28
// #define CrowPanel_35

#if defined(CrowPanel_35)
/*screen resolution*/
static const uint16_t screenWidth = 480;
static const uint16_t screenHeight = 320;
uint16_t calData[5] = { 353, 3568, 269, 3491, 7 }; /*touch caldata*/

#elif defined(CrowPanel_24)
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
uint16_t calData[5] = { 557, 3263, 369, 3493, 3 };

#elif defined(CrowPanel_28)
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;
uint16_t calData[5] = { 189, 3416, 359, 3439, 1 };
#endif


TFT_eSPI tft = TFT_eSPI(); /* TFT entity */

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[screenWidth * screenHeight / 13];

//_______________________
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {

  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();

  int16_t x1 = screenWidth - area->x2 - 1;
  int16_t y1 = screenHeight - area->y2 - 1;

  tft.setAddrWindow(x1, y1, w, h);

  for (uint32_t i = 0; i < w * h; i++) {
    uint16_t color = color_p[w * h - 1 - i].full;  // 倒序绘制
    tft.pushColor(color);
  }

  tft.endWrite();

  lv_disp_flush_ready(disp);
}

uint16_t touchX, touchY;
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  bool touched = tft.getTouch(&touchX, &touchY, 600);
  if (!touched) {
    data->state = LV_INDEV_STATE_REL;
  } else {
    data->state = LV_INDEV_STATE_PR;

    data->point.x = screenWidth - touchX - 1;
    data->point.y = screenHeight - touchY - 1;

    Serial.print("Data x: ");
    Serial.println(data->point.x);
    Serial.print("Data y: ");
    Serial.println(data->point.y);
  }
}


void setup() {
  Serial.begin(115200); /*serial init */

  //LCD init
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTouch(calData);
  delay(100);

  //lvgl init
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

  //旋转
  // disp_drv.sw_rotate = 1;  // If you turn on software rotation, Do not update or replace LVGL
  // disp_drv.rotated = LV_DISP_ROT_180;
  // disp_drv.full_refresh = 1;  // full_refresh must be 1

  /* Initialize the (dummy) input device driver */
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register(&indev_drv);

  tft.fillScreen(TFT_BLACK);

  // lv_demo_widgets();  // LVGL demo
  ui_init();

  Serial.println("Setup done");
}


void loop() {
  lv_timer_handler();
  lv_tick_inc(5);  //放置5ms过去
  delay(5);
}