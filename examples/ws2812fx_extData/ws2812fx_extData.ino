/*
  WS2812FX v1.41 introduced the ability to inject external data into some
  of the more advanced effects. This sketch demontrates how to do that.
  Note: by default, the "more advanced effects" are only setup to compile
  for microprocessors that have plentiful memory (like ESPs or RP2040s).
  The compile will fail if targetting an old, wimpy Arduino.
  
  Keith Lord - 2022

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
  2022-07-12 initial version
*/

#include <WS2812FX.h>

#define LED_PIN   12  // digital pin used to drive the LED strip
#define LED_COUNT 30  // number of LEDs on the strip

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// the oscillator effect produces two "oscillators" by default. We're going
// to inject our own Oscillator struct array that creates three oscillators.
Oscillator oscillators[] = { // 3 oscillators
  {LED_COUNT/4,              0,  1}, // size, pos, speed
  {LED_COUNT/4,  LED_COUNT / 2,  2},
  {LED_COUNT/4,  LED_COUNT - 1, -2}
};
uint16_t numOsc = sizeof(oscillators) / sizeof(oscillators[0]);

void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(64);

  // Here's where we'll instruct segment 0 to use our custom oscillator array
  // by setting an external data source. Note the setExtDataSrc() function expects
  // a (uint8_t*) pointer, so cast accordingly.
  // parameters: seg, pointer to external data array, number of data array elemenst
  ws2812fx.setExtDataSrc(0, (uint8_t*)&oscillators, numOsc);

  // parameters: seg, start, stop, mode, colors, speed
  ws2812fx.setSegment(0,  0,  LED_COUNT-1, FX_MODE_OSCILLATOR, COLORS(RED, GREEN, BLUE), 1000);

  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}
