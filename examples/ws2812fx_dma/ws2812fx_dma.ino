/*
  This sketch introduces the use of a custom show() function.
  It borrows the DMA class from the NeoPixelBus library to use the ESP8266's DMA
  channel to drive LED updates instead of the default Adafruit_NeoPixel "bit bang"
  method.
  
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
  2018-05-30 initial version
*/

#include <WS2812FX.h>
#include <NeoPixelBus.h>

#define LED_PIN    3  // digital pin used to drive the LED strip, (for ESP8266 DMA, must use GPIO3/RX/D9)
#define LED_COUNT 30  // number of LEDs on the strip

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// create a NeoPixelBus instance
NeoPixelBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> strip(LED_COUNT);

void setup() {
  Serial.begin(115200);

  ws2812fx.init();

  // MUST run strip.Begin() after ws2812fx.init(), so GPIO3 is initalized properly
  strip.Begin();
  strip.Show();

  // set the custom show function
  ws2812fx.setCustomShow(myCustomShow);

  ws2812fx.setBrightness(255);
  const uint32_t colors[] = {RED, BLACK, BLACK};
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_COMET, colors, 2000, NO_OPTIONS);

  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}

void myCustomShow(void) {
  if(strip.CanShow()) {
    // copy the WS2812FX pixel data to the NeoPixelBus instance
    memcpy(strip.Pixels(), ws2812fx.getPixels(), strip.PixelsSize());
    strip.Dirty();
    strip.Show();
  }
}


