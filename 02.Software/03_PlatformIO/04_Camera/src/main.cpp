#include <Arduino.h>
#include "esp_camera.h"

#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13

void cameraInit()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM; // 使用新的名称 pin_sccb_sda
  config.pin_sccb_scl = SIOC_GPIO_NUM; // 使用新的名称 pin_sccb_scl
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_camera_init(&config);

  if (esp_camera_init(&config) != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", esp_camera_init(&config)); // 初始化失败
    return;
  }
  else
  {
    Serial.println("Camera init");
  }

  delay(500);
}

void setup()
{

  Serial.begin(115200);

  cameraInit();
}

void loop()
{
  camera_fb_t *fb = esp_camera_fb_get(); // 获取帧缓冲
  if (!fb)
  {
    Serial.println("Failed to capture image"); // 无法捕获图片
    return;
  }

  if (fb->format != PIXFORMAT_JPEG)
  {
    Serial.println("no JPEG format"); // 非JPEG
  }
  else
  {
    Serial.println("Captured JPEG image");
  }

  esp_camera_fb_return(fb); // 释放缓冲
}
