/*
  Custom effect that creates two Larson scanners moving in opposite directions
  
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

#ifndef DualLarson_h
#define DualLarson_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t dualLarson(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  int seglen = seg->stop - seg->start + 1;

  static int16_t index = 0, dir = 1;
  index += dir;

  ws2812fx.fade_out();

  ws2812fx.setPixelColor(seg->start + index, seg->colors[0]);
  ws2812fx.setPixelColor(seg->stop  - index, seg->colors[2]);

  if(index >= (seg->stop - seg->start) || index <= 0) dir = -dir; 

  return (seg->speed / (seglen * 2));
}

#endif
