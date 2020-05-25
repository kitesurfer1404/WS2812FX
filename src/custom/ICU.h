/*
  Custom effect that mimics two eyes looking about
  
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
  2018-07-26 initial version
*/

#ifndef ICU_h
#define ICU_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t icu(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  uint16_t dest = segrt->counter_mode_step & 0xFFFF;
 
  ws2812fx.setPixelColor(seg->start + dest, seg->colors[0]);
  ws2812fx.setPixelColor(seg->start + dest + seglen/2, seg->colors[0]);

  if(segrt->aux_param3 == dest) { // pause between eye movements
    if(ws2812fx.random8(6) == 0) { // blink once in a while
      ws2812fx.setPixelColor(seg->start + dest, BLACK);
      ws2812fx.setPixelColor(seg->start + dest + seglen/2, BLACK);
      return 200;
    }
    segrt->aux_param3 = ws2812fx.random16(seglen/2);
    ws2812fx.setCycle();
    return 1000 + ws2812fx.random16(2000);
  }

  ws2812fx.setPixelColor(seg->start + dest, BLACK);
  ws2812fx.setPixelColor(seg->start + dest + seglen/2, BLACK);

  if(segrt->aux_param3 > segrt->counter_mode_step) {
    segrt->counter_mode_step++;
    dest++;
  } else if (segrt->aux_param3 < segrt->counter_mode_step) {
    segrt->counter_mode_step--;
    dest--;
  }

  ws2812fx.setPixelColor(seg->start + dest, seg->colors[0]);
  ws2812fx.setPixelColor(seg->start + dest + seglen/2, seg->colors[0]);

  return (seg->speed / seglen);
}

#endif
