/*
  Custom effect that looks like popcorn.
  The 'popcorn' is color[0] and the background color is color[1].
  If color[0] is the same as color[1], the 'popcorn' will be a random color.
  
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
  2018-11-13 initial version
*/

#ifndef Popcorn_h
#define Popcorn_h

#include <WS2812FX.h>

#define MAX_NUM_POPCORN 10
#define GRAVITY 0.1

extern WS2812FX ws2812fx;

typedef struct Kernel {
  float position;
  float velocity;
  int32_t color;
} kernel;

uint16_t popcorn(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  uint16_t seglen = seg->stop - seg->start + 1;
  uint32_t popcornColor = seg->colors[0];
  uint32_t bgColor = seg->colors[1];
  if(popcornColor == bgColor) popcornColor = ws2812fx.color_wheel(ws2812fx.random8());
  bool isReverse = (seg->options & REVERSE) != 0;

  static kernel popcorn[MAX_NUM_POPCORN];
  static float coeff = 0.0f;
  if(coeff == 0.0f) { // calculate the velocity coeff once (the secret sauce)
    coeff = pow((float)seglen, 0.5223324f) * 0.3944296f;
  }

  // reset all LEDs to background color
  for(uint16_t i=seg->start; i <= seg->stop; i++) {
    ws2812fx.setPixelColor(i, bgColor);
  }

  uint16_t ledIndex;
  for(int8_t i=0; i < MAX_NUM_POPCORN; i++) {
    bool isActive = popcorn[i].position >= 0.0f;

    if(isActive) { // if kernel is active, update its position
      popcorn[i].position += popcorn[i].velocity;
      popcorn[i].velocity -= GRAVITY;
      ledIndex = isReverse ? seg->stop - popcorn[i].position : seg->start + popcorn[i].position;
      if(ledIndex >= seg->start && ledIndex <= seg->stop) ws2812fx.setPixelColor(ledIndex, popcorn[i].color);
    } else { // if kernel is inactive, randomly pop it
      if(ws2812fx.random8() < 2) { // POP!!!
        popcorn[i].position = 0.0f;
        popcorn[i].velocity = coeff * (random(66, 100) / 100.0f);
        popcorn[i].color = popcornColor;
        ledIndex = isReverse ? seg->stop : seg->start;
        ws2812fx.setPixelColor(ledIndex, popcorn[i].color);
        ws2812fx.setCycle();
      }
    }
  }

  return(seg->speed / seglen);
}

#endif
