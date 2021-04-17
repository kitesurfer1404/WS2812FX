/*
  An implementation of Mark Kriegsman's TwinkleFOX effect
  (https://gist.github.com/kriegsman/756ea6dcae8e30845b5a)
  This implementation does not use the FastLED library.

  Colors[1] is the background color.
  If colors[0] is black, rainbow colors will be used, otherwise use colors[0].
  If colors[2] isn't black, use colors[0] and colors[2].
  Change the WS2812FX SIZE option to animate groups of LEDs.
  
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
  2019-11-25 initial version
*/

/*
Note: twinkleFOX is now one of the normal, built-in WS2812FX effects, so doesn't need to be
included as a custom effect. I left it here just for historical reference. Eventually it'll
be removed from the custom folder.
*/

#ifndef Twinklefox_h
#define Twinklefox_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint16_t twinkleFox(void) {
  uint16_t mySeed = 0; // reset the random number generator seed
  WS2812FX::Segment* seg = ws2812fx.getSegment(); // get the current segment
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();

  // Get and translate the segment's size option
  uint8_t size = 1 << ((seg->options >> 1) & 0x03); // 1,2,4,8

  // Get the segment's colors array values
  uint32_t color0 = seg->colors[0];
  uint32_t color1 = seg->colors[1];
  uint32_t color2 = seg->colors[2];
  uint32_t blendedColor;

  for (uint16_t i = seg->start; i <= seg->stop; i+=size) {
    // Use Mark Kriegsman's clever idea of using pseudo-random numbers to determine
    // each LED's initial and increment blend values
    mySeed = (mySeed * 2053) + 13849; // a random, but deterministic, number
    uint16_t initValue = (mySeed + (mySeed >> 8)) & 0xff; // the LED's initial blend index (0-255)
    mySeed = (mySeed * 2053) + 13849; // another random, but deterministic, number
    uint16_t incrValue = (((mySeed + (mySeed >> 8)) & 0x07) + 1) * 2; // blend index increment (2,4,6,8,10,12,14,16)

    // We're going to use a sine function to blend colors, instead of Mark's triangle
    // function, simply because a sine lookup table is already built into the
    // Adafruit_NeoPixel lib. Yes, I'm lazy.
    // Use the counter_mode_call var as a clock "tick" counter and calc the blend index
    uint8_t blendIndex = (initValue + (segrt->counter_mode_call * incrValue)) & 0xff; // 0-255
    // Index into the built-in Adafruit_NeoPixel sine table to lookup the blend amount
    uint8_t blendAmt = Adafruit_NeoPixel::sine8(blendIndex); // 0-255

    // If colors[0] is BLACK, blend random colors
    if(color0 == BLACK) {
      blendedColor = ws2812fx.color_blend(ws2812fx.color_wheel(initValue), color1, blendAmt);
    // If colors[2] isn't BLACK, choose to blend colors[0]/colors[1] or colors[1]/colors[2]
    // (which color pair to blend is picked randomly)
    } else if((color2 != BLACK) && (initValue < 128) == 0) {
      blendedColor = ws2812fx.color_blend(color2, color1, blendAmt);
    // Otherwise always blend colors[0]/colors[1]
    } else {
      blendedColor = ws2812fx.color_blend(color0, color1, blendAmt);
    }

    // Assign the new color to the number of LEDs specified by the SIZE option
    for(uint8_t j=0; j<size; j++) {
      if((i + j) <= seg->stop) {
        ws2812fx.setPixelColor(i + j, blendedColor);
      }
    }
  }
  ws2812fx.setCycle();
  return seg->speed / 32;
}

#endif