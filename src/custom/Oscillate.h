/*
  Custom effect that creates three color spans that oscillate back and forth.
  
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

#ifndef Oscillate_h
#define Oscillate_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

typedef struct Oscillator {
  int16_t pos;
  int8_t  size;
  int8_t  dir;
  int8_t  speed;
} oscillator;

uint16_t oscillate(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  int seglen = seg->stop - seg->start + 1;

  static oscillator oscillators[] = {
    {seglen/4,   seglen/8,  1, 1},
    {seglen/4*2, seglen/8, -1, 1},
    {seglen/4*3, seglen/8,  1, 2}
  };

  for(int8_t i=0; i < sizeof(oscillators)/sizeof(oscillators[0]); i++) {
    oscillators[i].pos += oscillators[i].dir * oscillators[i].speed;
    if((oscillators[i].dir == -1) && (oscillators[i].pos <= 0)) {
      oscillators[i].pos = 0;
      oscillators[i].dir = 1;
      oscillators[i].speed = random(1, 3);
      ws2812fx.setCycle();
    }
    if((oscillators[i].dir == 1) && (oscillators[i].pos >= (seglen - 1))) {
      oscillators[i].pos = seglen - 1;
      oscillators[i].dir = -1;
      oscillators[i].speed = random(1, 3);
      ws2812fx.setCycle();
    }
  }

  for(int16_t i=0; i < seglen; i++) {
    uint32_t color = BLACK;
    for(int8_t j=0; j < sizeof(oscillators)/sizeof(oscillators[0]); j++) {
      if(i >= oscillators[j].pos - oscillators[j].size && i <= oscillators[j].pos + oscillators[j].size) {
        color = (color == BLACK) ? seg->colors[j] : ws2812fx.color_blend(color, seg->colors[j], 128);
      }
    }
    ws2812fx.setPixelColor(seg->start + i, color);
  }
  return(seg->speed / 8);
}

#endif
