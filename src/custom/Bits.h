/*
  Custom effect that shows a bit pattern in a rainbow of colors


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
  2021-03-14 initial version (pi day!)
*/

#ifndef Bits_h
#define Bits_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

const char bitsData[]  = "1110101111"; // pi=3.14

uint16_t bits(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  int8_t colorIndex = segrt->aux_param++;
  uint8_t numBits = sizeof(bitsData) - 1;
  uint8_t ledsPerBit = seglen / (numBits * 2);
  //Serial.println(ledsPerBit);

  uint32_t color = ws2812fx.color_wheel(segrt->aux_param++); // rainbow of colors
  uint16_t bitIndex = 0;
  for(uint16_t i=seg->start; i < numBits * ledsPerBit * 2; i += (ledsPerBit * 2)) {
    for(uint16_t j=0; j < ledsPerBit; j++) { // bit
      if(bitsData[bitIndex] == '1') {
        ws2812fx.setPixelColor(i + j, color);
      }else {
        ws2812fx.setPixelColor(i + j, BLACK);
      }
    }
    for(uint16_t j=0; j < ledsPerBit; j++) { // space
      ws2812fx.setPixelColor(i + ledsPerBit + j, BLACK);
    }
    bitIndex++;
    if(bitIndex >= numBits) bitIndex = 0;
  }

  if(segrt->aux_param == 0) ws2812fx.setCycle();
  return seg->speed;
}

#endif
