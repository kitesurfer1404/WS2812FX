/*
  Custom effect that changes random pixels to transition between colors

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
  2018-08-19 initial version
*/

#ifndef BlockDissolve_h
#define BlockDissolve_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t blockDissolve(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  uint32_t color = seg->colors[segrt->aux_param];
  ws2812fx.setPixelColor(seg->start, color);
  // get the decimated color after setPixelColor() has mangled it
  // in accordance to the brightness setting
  uint32_t desColor = ws2812fx.getPixelColor(seg->start);

  for(uint16_t i=0; i<seglen; i++) {
    int index = seg->start + ws2812fx.random16(seglen);
    if(ws2812fx.getPixelColor(index) != desColor) {
      ws2812fx.setPixelColor(index, color);
      return seg->speed;
    }
  }

  for(uint16_t i=seg->start; i<seg->stop; i++) {
    ws2812fx.setPixelColor(i, color);
  }

  segrt->aux_param = (segrt->aux_param + 1) % MAX_NUM_COLORS;
  if(segrt->aux_param == 0) ws2812fx.setCycle();
  return seg->speed;
}

#endif
