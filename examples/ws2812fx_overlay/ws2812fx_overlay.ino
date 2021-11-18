/*
  Demo sketch which shows how to configure WS2812FX to overlay two
  virtual strips onto one physical strip. It's sort of a counterpart to
  the ws2812fx_virtual_strip sketch, which distributes one virtual strip
  across two physical strips.


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
  2020-09-10 initial version
*/

#include <WS2812FX.h>

#define LED_PIN     4
#define LED_COUNT 144

// Create instances of two virtual strips and one physical strip.
// The animations will each run independently on the virtual strips,
// but the LED data will be merged and copied to the physical strip.
WS2812FX ws2812fx_v1 = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800,1,1);
WS2812FX ws2812fx_v2 = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800,1,1);
WS2812FX ws2812fx_p  = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800,1,1);

void setup() {
  // Initialize the virtual strips
  ws2812fx_v1.init();
  ws2812fx_v1.setBrightness(64);
  ws2812fx_v1.setSegment(0, 0, LED_COUNT-1, FX_MODE_RAINBOW_CYCLE, BLACK, 1000, NO_OPTIONS);
  ws2812fx_v1.start();

  ws2812fx_v2.init();
  ws2812fx_v2.setBrightness(255);
  ws2812fx_v2.setSegment(0, 0, LED_COUNT-1, FX_MODE_LARSON_SCANNER, WHITE, 5000, NO_OPTIONS);
  ws2812fx_v2.start();

  // Initialize the physical strip. Since the physical strip gets
  // it's data from the virtual strips, no need to initialize segments.
  ws2812fx_p.init();

  // Config custom show() functions for the virtual strips, so their
  // pixel data gets merged and copied to the physical strip.
  ws2812fx_v1.setCustomShow(myCustomShow);
  ws2812fx_v2.setCustomShow(myCustomShow);
}

void loop() {
  // Run effects only on the virtual strips.
  ws2812fx_v1.service();
  ws2812fx_v2.service();
}

// The custom show() function blends and copies the virtual strips'
// data to the physical strip and then runs it's show() function.
void myCustomShow(void) {
  // get pointers to all the pixel data arrays
  uint8_t *pixels_v1 = ws2812fx_v1.getPixels();
  uint8_t *pixels_v2 = ws2812fx_v2.getPixels();
  uint8_t *pixels_p  = ws2812fx_p.getPixels();

  // blend the pixel data from the virtual strips and save it
  // to the physical strip
  for (uint16_t i=0; i < ws2812fx_p.getNumBytes(); i++) {
    pixels_p[i] = (pixels_v1[i] / 2) + (pixels_v2[i] / 2);
  }

  // Call the physical strip's show() function.
  // Note: the virtual strip's show() functions are never called.
  ws2812fx_p.Adafruit_NeoPixel::show();
}
