#include <Arduino.h>

#define LED_GPIO 2

void setup() {
  pinMode(LED_GPIO, OUTPUT);
}

void loop() {
  digitalWrite(LED_GPIO, !digitalRead(LED_GPIO));
  delay(1000);
}
