/*
  Custom effect that animates a 2D matrix of LEDs

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
  2020-04-12 initial version
*/

#ifndef Matrix_h
#define Matrix_h

#include <WS2812FX.h>

extern WS2812FX ws2812fx;

uint32_t matrix_leds[][3][8] = { // setup for a 3x8 LED matrix
  { // page 0
  {RED,   RED,   RED,   RED,   RED,   RED,   RED,   RED},   // row 0
  {WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE}, // row 1
  {BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE,  BLUE}   // row 2
  },
  { // page 1
  {YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW}, // row 0
  {PINK,   PINK,   PINK,   PINK,   PINK,   PINK,   PINK,   PINK},   // row 1
  {GREEN,  GREEN,  GREEN,  GREEN,  GREEN,  GREEN,  GREEN,  GREEN}   // row 2
  }
};

uint16_t matrix(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  const int numPages = sizeof(matrix_leds) / sizeof(matrix_leds[0]);
  const int numRows  = sizeof(matrix_leds[0]) / sizeof(matrix_leds[0][0]);
  const int numCols  = sizeof(matrix_leds[0][0]) / sizeof(matrix_leds[0][0][0]);
// Serial.print("numPages:"); Serial.println(numPages);
// Serial.print("numRows:"); Serial.println(numRows);
// Serial.print("numCols:"); Serial.println(numCols);

  uint16_t segIndex = 0;
  uint8_t pageIndex = segrt->aux_param; // aux_param will store the page index
  for(int i=0; i<numRows; i++) {
    for(int j=0; j<numCols; j++) {
      if(segIndex < seglen) {
        ws2812fx.setPixelColor(segIndex, matrix_leds[pageIndex][i][j]);
        segIndex++;
      }
    }
  }

  // increment to the next page
  segrt->aux_param < (numPages - 1) ? segrt->aux_param++ : segrt->aux_param = 0;
  if(segrt->aux_param == 0) ws2812fx.setCycle();
  return seg->speed;
}

#endif
