/*
  WS2812FX segment sequence demo. Creates a list of segments and dynamically
  changes which segments are active.

  FEATURES
    * example of creating idle segments


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2020  Keith Lord 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
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
  2020-05-16 initial version
  
*/

#include <WS2812FX.h>

#define LED_PIN    10  // digital pin used to drive the LED strip
#define LED_COUNT 144  // number of LEDs on the strip

#define NUM_SEGMENTS        4  // maximum total number of segments that can be created
#define NUM_ACTIVE_SEGMENTS 2  // maximum number of segments that can be actively running

// create helper macros that define the start and end LEDs for each of our two segments
#define LOWER_SEG_RANGE 0, LED_COUNT/2 - 1
#define UPPER_SEG_RANGE LED_COUNT/2, LED_COUNT - 1

// example of the new WS2812FX constructor that adds parameters for the number of
// segments allowed and the number of active segments allowed.
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, NUM_SEGMENTS, NUM_ACTIVE_SEGMENTS);

void setup() {
  Serial.begin(115200);

  ws2812fx.init();
  ws2812fx.setBrightness(32);

  // create two active segments
  ws2812fx.setSegment(0, LOWER_SEG_RANGE, FX_MODE_BLINK, YELLOW,             2000, NO_OPTIONS);
  ws2812fx.setSegment(1, UPPER_SEG_RANGE, FX_MODE_BLINK, COLORS(RED, GREEN), 2000, NO_OPTIONS);

  // create additional "idle" segments that will be activated later
  ws2812fx.setIdleSegment(2, LOWER_SEG_RANGE, FX_MODE_BLINK, CYAN,               2000, NO_OPTIONS);
  ws2812fx.setIdleSegment(3, UPPER_SEG_RANGE, FX_MODE_BLINK, COLORS(PINK, BLUE), 2000, NO_OPTIONS);

  ws2812fx.start();
}

void loop() {
  static unsigned long timer = millis();
  static unsigned int  blink_counter = 0;

  ws2812fx.service();

  /* the lower segment is updated based on a timer.
     the lower segment will change every 10 seconds
  */
  if(millis() > timer + 10000) {        // every 10 seconds...
    if(ws2812fx.isActiveSegment(0)) {   // if seg[0] is active, switch to seg[2]
      ws2812fx.swapActiveSegment(0, 2);
    } else {                            // else, switch to seg[0]
      ws2812fx.swapActiveSegment(2, 0);
    }
    timer = millis();
  }

  /* the upper segment is updated based on counting animation cycles. One "cycle" for the Blink
   *  effect is one on/off sequence. We'll change the upper segment every 5 blinks.
   */
  // increment the blink counter every time seg[1] or seg[3] complete an animation cycle
  if(ws2812fx.isCycle(1) || ws2812fx.isCycle(3)) {
    blink_counter++;
  }
  if(blink_counter >= 5) {                  // every 5 blinks...
    if(ws2812fx.isActiveSegment(1)) {       // if seg[1] active, switch to seg[3]
      ws2812fx.swapActiveSegment(1, 3);
    } else {                                // else, switch to seg[1]
      ws2812fx.swapActiveSegment(3, 1);
    }
    blink_counter = 0;
  }
}
