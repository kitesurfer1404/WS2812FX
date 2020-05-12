/*
  Custom effect that mimics a multiband VU meter
  
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
  2018-08-21 initial version
*/

#ifndef VUMeter_h
#define VUMeter_h

#include <WS2812FX.h>

#ifndef NUM_BANDS
  #define NUM_BANDS 2
#endif

// set USE_RANDOM_DATA to false in the line below if vuMeterBands[] is populated
// by an external data source. otherwise random data will be used for the effect.
#ifndef USE_RANDOM_DATA
  #define USE_RANDOM_DATA true
#endif

extern WS2812FX ws2812fx;

uint8_t vuMeterBands[NUM_BANDS]; // global VU meter band amplitude data (range 0-255)

uint16_t vuMeter(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  uint16_t seglen = seg->stop - seg->start + 1;
  uint16_t bandSize = seglen / NUM_BANDS;

  for(uint8_t i=0; i<NUM_BANDS; i++) {
#if USE_RANDOM_DATA
    int randomData = vuMeterBands[i] + ws2812fx.random8(32) - ws2812fx.random8(32);
    vuMeterBands[i] = (randomData < 0 || randomData > 255) ? 128 : randomData;
#endif

    uint8_t scaledBand = (vuMeterBands[i] * bandSize) / 256;
    for(uint16_t j=0; j<bandSize; j++) {
      uint16_t index = seg->start + (i * bandSize) + j;
      if(j <= scaledBand) {
        if(j < bandSize - 4) ws2812fx.setPixelColor(index, seg->colors[0]);
        else if(j < bandSize - 2) ws2812fx.setPixelColor(index, seg->colors[1]);
        else ws2812fx.setPixelColor(index, seg->colors[2]);
      } else {
        ws2812fx.setPixelColor(index, BLACK);
      }
    }
  }
  ws2812fx.setCycle();

  return seg->speed;
}

#endif
