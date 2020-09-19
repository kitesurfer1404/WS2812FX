/*
  Demo sketch which shows how to configure WS2812FX such that two physical
  strips of LEDs, driven by two separate GPIO pins, get their pixel data
  from one virtual strip. One effect can be spread across two strips. It's
  sort of a counterpart to the ws2812fx_overlay sketch, which overlays two
  virtual strips onto one physical strip.


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2020  Keith Lord 

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
  2020-05-29 initial version
*/

#include <WS2812FX.h>

#define LED_PIN_V1     10  // virtual digital pin used to drive the virtual LED strip
#define LED_PIN_P1     10  // physical digital pin used to drive the first physical LED strip
#define LED_PIN_P2     11  // physical digital pin used to drive the second physical LED strip
#define LED_COUNT_P1   30  // number of LEDs on the first physical strip
#define LED_COUNT_P2   30  // number of LEDs on the second physical strip

// create an instance of one virtual strip and two physical strips.
// the physical strips are only initialized with a count of one LED, since
// these strips will ultimately use the pixel data of the virtual strip.
// (Note the instances are created with support of only one segment and one
// segment_runtime, just so the sketch fits in an Arduino's limited SRAM.)
WS2812FX ws2812fx_v1 = WS2812FX(LED_COUNT_P1 + LED_COUNT_P2, LED_PIN_V1, NEO_GRB + NEO_KHZ800, 1, 1);
WS2812FX ws2812fx_p1 = WS2812FX(1,                           LED_PIN_P1, NEO_GRB + NEO_KHZ800, 1, 1);
WS2812FX ws2812fx_p2 = WS2812FX(1,                           LED_PIN_P2, NEO_GRB + NEO_KHZ800, 1, 1);

void setup() {
  // initialize the virtual strip as you would any normal ws2812fx instance
  ws2812fx_v1.init();
  ws2812fx_v1.setBrightness(255);
  ws2812fx_v1.setSegment(0, 0, ws2812fx_v1.getLength()-1, FX_MODE_COMET, RED, 2000, NO_OPTIONS);
  ws2812fx_v1.start();

  // init the physical strip's GPIOs and reassign their pixel data
  // pointer to use the virtual strip's pixel data.
  ws2812fx_p1.init();
  ws2812fx_p1.setPixels(LED_COUNT_P1, ws2812fx_v1.getPixels());
  ws2812fx_p2.init();
  ws2812fx_p2.setPixels(LED_COUNT_P2, ws2812fx_v1.getPixels() + (LED_COUNT_P1 * ws2812fx_v1.getNumBytesPerPixel()));

  // config a custom show() function for the virtual strip, so pixel
  // data gets sent to the physical strips's LEDs instead
  ws2812fx_v1.setCustomShow(myCustomShow);
}

void loop() {
  // update the virtual strip's pixel data by calling service() as you normally would
  ws2812fx_v1.service();
}

// update the physical strips's LEDs
void myCustomShow(void) {
  ws2812fx_p1.show();
  ws2812fx_p2.show();
}
