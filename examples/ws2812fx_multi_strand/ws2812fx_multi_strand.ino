/*
  Demo sketch which showcases spanning a single WS2812FX animation across
  multiple physical LED strips.
  This is done by instantiating an Adafruit_NeoPixel for each physical strip,
  instantiating a virtual WS2812FX, and setting the custom show on the virtual
  WS2812FX which copies the pixel memory from the virtual strip to the phyiscal
  strips.

  The physical strips do not need to be the identical length, this is done here
  for simplicity.

  Keith Lord - 2020

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
  2020-09-07 initial version
*/

#include <WS2812FX.h>
#include <Adafruit_NeoPixel.h>

#define PIXELDENSITY 60 // 60 pixels per meter
#define STRIP_LENGTH 1  // Meters
#define TOTAL_STRIPS 3  // Count of LED Strips

// Strip pins
#define STRIP_1_PIN 3
#define STRIP_2_PIN 4
#define STRIP_3_PIN 5

#define FAKE_PIN -1

// Create the three neopixels representing the physical strips
auto strip1 = Adafruit_NeoPixel(STRIP_LENGTH * PIXELDENSITY, STRIP_1_PIN, NEO_RGB + NEO_KHZ800);
auto strip2 = Adafruit_NeoPixel(STRIP_LENGTH * PIXELDENSITY, STRIP_2_PIN, NEO_RGB + NEO_KHZ800);
auto strip3 = Adafruit_NeoPixel(STRIP_LENGTH * PIXELDENSITY, STRIP_3_PIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel *physicalStrips[] = {&strip1, &strip2, &strip3};

// Create a virtual WS2812FX to run the animations
// The length of the virtual strip must be the combination of all physical strip lengths.
WS2812FX ws2812fx = WS2812FX(STRIP_LENGTH * PIXELDENSITY * TOTAL_STRIPS, FAKE_PIN, NEO_RGB + NEO_KHZ800);

// Copies the pixels from the animation strip to the physical strips
// then shows the physical strip
void showPhysicalStripsFromVirualStrip(Adafruit_NeoPixel *virtualStrip, Adafruit_NeoPixel *strips[], uint8_t stripsSize)
{
  // Copy pixel values from virtual strip to physical strips
  uint8_t *pixelPointer = ws2812fx.getPixels();
  for (int stripIndex = 0; stripIndex < stripsSize; ++stripIndex)
  {
    Adafruit_NeoPixel strip = *strips[stripIndex];
    memcpy(strip.getPixels(), pixelPointer, strip.numPixels());
    pixelPointer = pixelPointer + strip.numPixels();
  }

  // Show physical strips
  for (int stripIndex = 0; stripIndex < stripsSize; ++stripIndex)
  {
    strips[stripIndex]->show();
  }
}

void myCustomShow(void)
{
  showPhysicalStripsFromVirualStrip(&ws2812fx, physicalStrips, TOTAL_STRIPS);
}

void setup()
{
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setMode(FX_MODE_CHASE_RAINBOW);
  ws2812fx.start();

  // Set the custom show function
  ws2812fx.setCustomShow(myCustomShow);
}

void loop()
{
  ws2812fx.service();
}
