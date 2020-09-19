/*
  Demo sketch which shows how to use a custom show() function to drive LEDs
  that implement a synchronous protocol (i.e. DotStar/APA102 LEDs) using the
  ESP8266's SPI hardware.

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
  2020-09-17 initial version
*/

#include <WS2812FX.h>
#include <SPI.h>

#define LED_COUNT 144
#define DATA_PIN   13  // MOSI/GPIO13/D7
#define CLOCK_PIN  14  // SCLK/GPIO14/D5

// Create the global ws2812fx instance.
// Note: APA102 LEDs expect the pixel data in blue/green/red order, so use
// the NEO_BGR flag. For other types of LEDs change it accordingly.
WS2812FX ws2812fx = WS2812FX(LED_COUNT, DATA_PIN, NEO_BGR + NEO_KHZ800);

void setup() {
  // init the ws2812fx instance
  ws2812fx.init();
  ws2812fx.setBrightness(32);
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_DUAL_SCAN, BLUE, 2000, NO_OPTIONS);
  ws2812fx.start();

  // config a custom show() function, so pixel data gets pushed out the SPI interface
  ws2812fx.setCustomShow(myCustomShow);

  // init the SPI hardware after setting up the ws2812fx instance
  SPI.begin();
}

void loop() {
  ws2812fx.service();
}

void myCustomShow(void) {
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  SPI.transfer(0x00); // send the start frame
  SPI.transfer(0x00);
  SPI.transfer(0x00);
  SPI.transfer(0x00);

  // send the pixel data
  uint8_t *pixels =  ws2812fx.getPixels();
  for(uint16_t i=0; i < ws2812fx.numPixels(); i++) {
    SPI.transfer(0xff); // control byte (0xe0 + 5-bit brightness)
    SPI.transfer(*pixels++); // blue
    SPI.transfer(*pixels++); // green
    SPI.transfer(*pixels++); // red
  }

  SPI.transfer(0xff); // send the end frame
  SPI.transfer(0xff);
  SPI.transfer(0xff);
  SPI.transfer(0xff);
  SPI.endTransaction();
}
