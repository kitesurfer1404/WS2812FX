/*
  Custom effect that fades between three colors. You can add the TRIFADE_BLACK option
  in the setSegment() call to fade to black between each color.
  
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

#ifndef TriFade_h
#define TriFade_h

#define TRIFADE_BLACK (uint8_t)0b10000000 // same as REVERSE macro

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t triFade(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();

  uint8_t options = seg->options;
  bool trifade_black = (options & TRIFADE_BLACK) == TRIFADE_BLACK;

  uint16_t count = segrt->aux_param3;
  uint32_t color1 = 0, color2 = 0;

  if(count < 1536) {
    color1 = trifade_black ? BLACK: seg->colors[2];
    color2 = seg->colors[0];
  }
  if(count < 1280) {
    color1 = trifade_black ? seg->colors[2] : seg->colors[1];
    color2 = trifade_black ? BLACK : seg->colors[2];
  }
  if(count < 1024) {
    color1 = trifade_black ? BLACK : seg->colors[0];
    color2 = trifade_black ? seg->colors[2] : seg->colors[1];
  }
  if(count < 768) {
    color1 = trifade_black ? seg->colors[1] : seg->colors[2];
    color2 = trifade_black ? BLACK : seg->colors[0];
  }
  if(count < 512) {
    color1 = trifade_black ? BLACK : seg->colors[1];
    color2 = trifade_black ? seg->colors[1] : seg->colors[2];
  }
  if(count < 256) {
    color1 = seg->colors[0];
    color2 = trifade_black ? BLACK : seg->colors[1];
  }

  uint32_t color = ws2812fx.color_blend(color1, color2, count % 256);
  for(uint16_t i=seg->start; i <= seg->stop; i++) {
    ws2812fx.setPixelColor(i, color);
  }

  segrt->aux_param3 += 4;
  if(segrt->aux_param3 >= 1536) {
    segrt->aux_param3 = 0;
    ws2812fx.setCycle();
  }

  return (seg->speed / 128);
}

#endif
