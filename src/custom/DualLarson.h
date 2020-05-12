/*
  Custom effect that creates two Larson scanners moving in opposite directions.
  If you set the REVERSE option, an offset will be added to the comet after each
  cycle (so if the LEDs are arranged in a circle, the animation will appear to
  "walk" around the circle.)
  
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
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  static int16_t offset = 0;
  int8_t dir = segrt->aux_param ? -1 : 1;
  segrt->aux_param3 += dir;

  ws2812fx.fade_out();

  int16_t segIndex = (segrt->aux_param3 + offset) % seglen;
  ws2812fx.setPixelColor(seg->start + segIndex, seg->colors[0]);
  ws2812fx.setPixelColor(seg->stop  - segIndex, seg->colors[2]);

  if(segrt->aux_param3 >= (seg->stop - seg->start) || segrt->aux_param3 <= 0) {
    segrt->aux_param = !segrt->aux_param;
    if(seg->options & REVERSE) offset = (offset + 1) % seglen;
    if(!segrt->aux_param) ws2812fx.setCycle();
  }

  return (seg->speed / (seglen * 2));
}

#endif
