/*
  Demo sketch which uses a MSGEQ7 chip to create a 7 band spectrum analyzer
  See the SparkFun page: https://www.sparkfun.com/products/10468
  
  Keith Lord - 2018

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2018  Keith Lord 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  
  CHANGELOG
  2018-10-107 initial version
*/

#include <WS2812FX.h>

#define LED_COUNT 140
#define LED_PIN   4

// include and config the VUMeter custom effect
#define NUM_BANDS 7
#define USE_RANDOM_DATA false
#include "custom/VUMeter.h"

// MSGEQ7 pin assignments
#define STROBE 5
#define RESET  6
#define OUT    A0
#define MSGEQ7_DELAY 50

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  // initialize the MSGEQ7
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(OUT, INPUT);
  digitalWrite(RESET, LOW);
  digitalWrite(STROBE, HIGH);
  delay(1);

  // reset the MSGEQ7's output multiplexer
  digitalWrite(RESET, HIGH);
  delayMicroseconds(50);
  digitalWrite(RESET, LOW);
  delay(1);

  ws2812fx.init();
  ws2812fx.setBrightness(255);

  // setup the custom effect
  uint32_t colors[] = {GREEN, YELLOW, RED};
  uint8_t vuMeterMode = ws2812fx.setCustomMode(F("VU Meter"), vuMeter);
  ws2812fx.setSegment(0, 0, LED_COUNT-1, vuMeterMode, colors, MSGEQ7_DELAY, NO_OPTIONS);

  ws2812fx.start();
}

void loop() {
  static long then = 0;
  long now = millis();

  // read the MSGEQ7 and update the vuMeter custom effect's values
  // every MSGEQ7_DELAY ms
  if(now > then + MSGEQ7_DELAY) {
    for (uint8_t i=0; i < NUM_BANDS; i++) {
      digitalWrite(STROBE, LOW);
      delayMicroseconds(50);
      uint16_t value = analogRead(OUT); // read the MSGEQ7 value
      vuMeterBands[i] = value < 40 ? 0 : map(value, 40, 1023, 0, 255); // scale the value
      digitalWrite(STROBE, HIGH);
      delayMicroseconds(50);
    }
    then = now;
  }

  ws2812fx.service();
}
