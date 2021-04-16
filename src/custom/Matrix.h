/*
  Custom effect that animates a 2D matrix of LEDs

  In your sketch create an array of led color data:
  uint32_t matrix_leds[][3][8] = {
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

  Then tell the matrix effect about your array:
  configMatrix(NUM_PAGES, NUM_ROWS, NUM_COLS, (uint32_t*)matrix_leds);

  Then setup your matrix effect with setCustomMode() like you would any other
  custom effect.

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

struct Matrix {
  int8_t   numPages;
  int8_t   numRows;
  int8_t   numCols;
  uint32_t* colors;
};
struct Matrix _matrix; // global variable, so this custom effect shouldn't be used in more then one segment

void configMatrix(uint8_t numPages, uint8_t numRows, uint8_t numCols, uint32_t* colors) {
  _matrix.numPages = numPages;
  _matrix.numRows = numRows;
  _matrix.numCols = numCols;
  _matrix.colors = colors;
}

uint16_t matrix(void) {
  WS2812FX::Segment* seg = ws2812fx.getSegment();
  WS2812FX::Segment_runtime* segrt = ws2812fx.getSegmentRuntime();
  int seglen = seg->stop - seg->start + 1;

  uint16_t segIndex = seg->start;
  uint8_t pageIndex = segrt->aux_param * _matrix.numRows * _matrix.numCols; // aux_param will store the page index
  for(int rowIndex=0; rowIndex < _matrix.numRows; rowIndex++) {
    uint16_t matrixIndex = pageIndex + (rowIndex * _matrix.numCols);
    for(int colIndex=0; colIndex < _matrix.numCols; colIndex++) {
      if(segIndex <= seg->stop) {
        ws2812fx.setPixelColor(segIndex, _matrix.colors[matrixIndex + colIndex]);
        segIndex++;
      }
    }
  }

  // increment to the next page
  segrt->aux_param < (_matrix.numPages - 1) ? segrt->aux_param++ : segrt->aux_param = 0;
  if(segrt->aux_param == 0) ws2812fx.setCycle();
  return seg->speed;
}

#endif
