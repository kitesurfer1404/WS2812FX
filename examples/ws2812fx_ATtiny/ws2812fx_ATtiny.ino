/*
  WS2812FX ATtiny demo.
  
  Keith Lord - 2024
  
  FEATURES
    * example of WS2812FX using an ATtiny processor.
      Note the ATtiny is extremely memory constrained. Consequently
      processors with less than 4k of flash memory are not supported.
      Processors with 4k of flash memory only support the following
      seven effects:
        FX_MODE_STATIC
        FX_MODE_BLINK
        FX_MODE_STROBE
        FX_MODE_COLOR_WIPE
        FX_MODE_COLOR_WIPE_REV
        FX_MODE_TRICOLOR_CHASE
        FX_MODE_SPARKLE
      Processors with only 256 bytes of RAM can support a maximum of
      about 40 LEDs.


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2024  Keith Lord 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
  2024-05-20 initial version
  
*/

#include <WS2812FX.h>

#define LED_PIN   PIN_PA1  // digital pin used to drive the LED strip
#define LED_COUNT      32  // number of LEDs on the strip

byte pixelArray[LED_COUNT * 3];  // the ATtiny library requires the pixel array be statically allocated
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, pixelArray);

void setup() {
  pinMode(LED_PIN, OUTPUT); // the ATtiny library requires setting the LED GPIO pinMode 

//ws2812fx.init();  // running the init() function is not required for the ATtiny library
  ws2812fx.setBrightness(8);

  // parameters: index, start, stop, mode, color, speed, reverse
  ws2812fx.setSegment(0,  0, LED_COUNT - 1, FX_MODE_BLINK, GREEN); // the ATtiny library only allows one segment

  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}
