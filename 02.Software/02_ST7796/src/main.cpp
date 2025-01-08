#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void displayInit() {
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.fillRect(10,10,10,10,TFT_GREEN);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  displayInit();
}

void loop() {
}