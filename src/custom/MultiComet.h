/*
  Custom effect that creates random comets
  
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

#ifndef MultiComet_h
#define MultiComet_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t multiComet(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  int seglen = seg->stop - seg->start + 1;

  bool isReverse = (seg->options & REVERSE) == REVERSE;

  ws2812fx.fade_out();

  static uint16_t comets[] = {UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX, UINT16_MAX};
  static int8_t numComets = sizeof(comets)/sizeof(comets[0]);

  for(uint8_t i=0; i < numComets; i++) {
    if(comets[i] < seglen) {
      if(isReverse) {
        ws2812fx.setPixelColor(seg->stop - comets[i],  i % 2 ? seg->colors[0] : seg->colors[2]);
      } else {
        ws2812fx.setPixelColor(seg->start + comets[i], i % 2 ? seg->colors[0] : seg->colors[2]);
      }
      comets[i]++;
    } else {
      if(!random(seglen)) {
        comets[i] = 0;
        ws2812fx.setCycle();
      }
    }
  }

  return (seg->speed / seglen);
}

#endif
