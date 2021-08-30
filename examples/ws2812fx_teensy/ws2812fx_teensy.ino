/*
  This sketch is a modification of the ws2812fx_dma sketch, which uses
  Paul Stoffregen's WS2812Serial library (instead of Michael Miller's
  NeoPixelBus library) to enable the Teensy microcontroller's DMA to
  drive the LEDs.
  
  Note the WS2812Serial lib isn't installable from the Teensyduino IDE's
  Manage Libraries menu, but instead must be installed using the
  Sketch -> Include Library -> Add .ZIP Library after downloading
  WS2812Serial from Paul's GitHub page:
  https://github.com/PaulStoffregen/WS2812Serial
  
  Keith Lord - 2021

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2021  Keith Lord 

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
  2021-08-20 initial version
*/

#include <WS2812FX.h>
#include <WS2812Serial.h>

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33
//   Teensy 4.0:  1, 8, 14, 17, 20, 24, 29, 39
//   Teensy 4.1:  1, 8, 14, 17, 20, 24, 29, 35, 47, 53
#define LED_PIN     1  // digital pin used to drive the LED strip
#define LED_COUNT 144  // number of LEDs on the strip

// allocate a display buffer for WS2812Serial
DMAMEM byte displayMemory[LED_COUNT * 12]; // 12 bytes per RGB LED or 16 bytes per RGBW LED

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

WS2812Serial *ws2812serialRef;

void setup() {
  Serial.begin(115200);

  ws2812fx.init();
  ws2812fx.setBrightness(32);
  
  const uint32_t colors[] = {GREEN, BLACK, BLACK};
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_LARSON_SCANNER, colors, 5000, NO_OPTIONS);

  ws2812fx.setCustomShow(myCustomShow); // set the custom show function
  ws2812fx.start();

  // Instantiate and init the WS2812Serial instance last, so the LED_PIN pin is configured properly.
  // For the WS2812Serial instance always use WS2812_BGR, since it's sense of color byte order seems backwards.
  ws2812serialRef = new WS2812Serial(LED_COUNT, displayMemory, ws2812fx.getPixels(), LED_PIN, WS2812_BGR);
  ws2812serialRef->begin();
}

void loop() {
  ws2812fx.service();
}

void myCustomShow(void) {
  ws2812serialRef->show(); // run the WS2812Serial show() function
}
