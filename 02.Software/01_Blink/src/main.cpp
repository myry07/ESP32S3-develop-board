#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define LED_PIN 48  
#define LED_COUNT 1  

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void flashLED(void *param) {
  strip.begin();
  strip.show();

  while(1) {
    // r
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  vTaskDelay(1000);

  // g
  strip.setPixelColor(0, strip.Color(0, 255, 0));
  strip.show();
  vTaskDelay(1000);

  // b
  strip.setPixelColor(0, strip.Color(0, 0, 255));
  strip.show();
  vTaskDelay(1000);
  }
}

void setup() {
  xTaskCreate(flashLED, "flash", 8192, NULL, 1, NULL);
}

void loop() {
  // put your main code here, to run repeatedly:
}
