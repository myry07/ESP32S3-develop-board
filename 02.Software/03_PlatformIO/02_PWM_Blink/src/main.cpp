#include <Arduino.h>

#define LED_PIN 2 

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // 逐渐变亮
  for (int brightness = 0; brightness <= 255; brightness++) {
    analogWrite(LED_PIN, brightness);  // 设置 PWM 占空比
    delay(10);  // 调整延时以控制呼吸速度
  }

  // 逐渐变暗
  for (int brightness = 255; brightness >= 0; brightness--) {
    analogWrite(LED_PIN, brightness);  // 设置 PWM 占空比
    delay(10);  // 调整延时以控制呼吸速度
  }
}