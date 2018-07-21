/*
  Demo sketch for creating multiple custom effects.
  
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
  2018-07-19 initial version
*/

#include <WS2812FX.h>

// include the custom effects
#include "custom/DualLarson.h"
#include "custom/RandomChase.h"

#define LED_COUNT 30
#define LED_PIN 5

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  
  ws2812fx.init();
  ws2812fx.setBrightness(255);

  // setup the custom effects
  uint8_t randomChaseMode = ws2812fx.setCustomMode(F("Random Chase"), randomChase);
  uint8_t dualLarsonMode  = ws2812fx.setCustomMode(F("Dual Larson"), dualLarson);

  uint32_t colors[] = {RED, BLUE, WHITE};

  ws2812fx.setSegment(0, 0,           LED_COUNT/2 - 1, randomChaseMode, BLUE,     50, NO_OPTIONS);
  ws2812fx.setSegment(1, LED_COUNT/2, LED_COUNT - 1,   dualLarsonMode,  colors, 2000, FADE_SLOW);

  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}
