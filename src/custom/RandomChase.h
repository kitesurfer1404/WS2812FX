/*
  Custom effect that creates a random chase effect
  
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
  2018-02-26 initial version
*/

#ifndef RandomChase_h
#define RandomChase_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t randomChase(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  ws2812fx.copyPixels(seg->start + 1, seg->start, seglen - 1);
  uint32_t color = ws2812fx.getPixelColor(seg->start + 1);
  int r = random(6) != 0 ? (color >> 16 & 0xFF) : random(256);
  int g = random(6) != 0 ? (color >> 8  & 0xFF) : random(256);
  int b = random(6) != 0 ? (color       & 0xFF) : random(256);
  ws2812fx.setPixelColor(seg->start, r, g, b);

  if((segrt->counter_mode_call % seglen) == 0) ws2812fx.setCycle();
  return seg->speed;
}

#endif
