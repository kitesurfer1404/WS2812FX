/*
  Custom effect of "drops" filling a "glass"
  Note, this custom effect takes advantage of REVERSE, FADE and SIZE options.
  You should specify at least two colors in setSegment(), otherwise the second
  half of the animation will simply show the color BLACK (all LEDs off). Hint,
  split your strip into two segments and set the second segment's REVERSE option.
  
  Keith Lord - 2019

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2019  Keith Lord 

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
  2019-03-6 initial version
*/

#ifndef Fillerup_h
#define Fillerup_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t fillerup(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;
  bool isReverse = (seg->options & REVERSE) == REVERSE;

  // set the forground and background colors
  uint32_t fgColor = seg->colors[0];
  uint32_t bgColor = seg->colors[1];
  if(segrt->aux_param) { // swap colors
    fgColor = seg->colors[1];
    bgColor = seg->colors[0];
  }

  // run the standard comet effect
  ws2812fx.fade_out(bgColor);
  if(isReverse) {
    ws2812fx.setPixelColor(seg->stop  - segrt->counter_mode_step, fgColor);
  } else {
    ws2812fx.setPixelColor(seg->start + segrt->counter_mode_step, fgColor);
  }
  segrt->counter_mode_step = (segrt->counter_mode_step + 1) % seglen;

  // when drop reaches the fill line, incr the fill line
  if(segrt->counter_mode_step >= seglen - segrt->aux_param3 - 1) {
    segrt->aux_param3 += ((seg->options >> 1) & 3) + 1; // increment fill line by SIZE
    segrt->counter_mode_step = 0;
  }

 // repaint the "filled" portion every time
  for(uint8_t i=0; i < segrt->aux_param3; i++) {
    if(isReverse) {
      ws2812fx.setPixelColor(seg->start + i, fgColor);
    } else {
      ws2812fx.setPixelColor(seg->stop - i,  fgColor);
    }
  }

  // when "glass" is full, flip the colors and start again
  if(segrt->aux_param3 >= seglen) {
    segrt->aux_param = !segrt->aux_param;
    segrt->aux_param3 = 0;
    ws2812fx.setCycle();
  }

  return (seg->speed / seglen);
}

#endif
