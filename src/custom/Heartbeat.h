/*
  Custom effect that creates a pulsing, heartbeat effect. It applies the
  FADE_RATE and SIZE options.
  
  Keith Lord - 2020

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2020  Keith Lord 

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
  2020-01-29 initial version
*/

#ifndef Heartbeat_h
#define Heartbeat_h

#define BEATS_PER_MINUTE 50
#define MS_PER_BEAT (60000 / BEATS_PER_MINUTE)
#define SECOND_BEAT (MS_PER_BEAT / 3)

#include <WS2812FX.h>

extern WS2812FX ws2812fx;
void beatIt(WS2812FX::Segment*, uint8_t);

uint16_t heartbeat(void) {
  static unsigned long lastBeat = 0;
  static bool secondBeatActive = false;

  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  int seglen = seg->stop - seg->start + 1;

  // Get and translate the segment's size option
  uint8_t size = 2 << ((seg->options >> 1) & 0x03); // 2,4,8,16

  // copy pixels from the middle of the segment to the edges
  uint16_t bytesPerPixelBlock = size * ws2812fx.getNumBytesPerPixel();
  uint16_t centerOffset = (seglen / 2) * ws2812fx.getNumBytesPerPixel();
  uint16_t byteCount = centerOffset - bytesPerPixelBlock;
  memmove(ws2812fx.getPixels(), ws2812fx.getPixels() + bytesPerPixelBlock, byteCount);
  memmove(ws2812fx.getPixels() + centerOffset + bytesPerPixelBlock, ws2812fx.getPixels() + centerOffset, byteCount);

  ws2812fx.fade_out();

  unsigned long beatTimer = millis() - lastBeat;
  if((beatTimer > SECOND_BEAT) && !secondBeatActive) { // time for the second beat?
    beatIt(seg, size); // create the second beat
    secondBeatActive = true;
  }
  if(beatTimer > MS_PER_BEAT) { // time to reset the beat timer?
    beatIt(seg, size); // create the first beat
    secondBeatActive = false;
    lastBeat = millis();
    ws2812fx.setCycle();
  }

  return(seg->speed / 32);
}

// light up ('size' * 2) LEDs in the middle of the segment (starts a beat)
void beatIt(WS2812FX::Segment* seg, uint8_t size) {
  int seglen = seg->stop - seg->start + 1;
  uint16_t startLed = seg->start + (seglen / 2) - size;
  for (uint16_t i = startLed; i < startLed + (size * 2); i++) {
    ws2812fx.setPixelColor(i, seg->colors[0]);
  }
}
#endif