/*
  A demo of effect transitions.
  A new class WS2812FXT (note the 'T' at the end) provides the ability to
  create effect transitions. Note, WS2812FXT essentially creates THREE
  WS2812FX instances to do it's work, so uses considerably more memory
  than the normal WS2812FX class.

  WS2812FXT creates three WS2812FX objects, two virtual LED strips (v1 and v2)
  and one physical strip (p1). You setup the two virtual strips using the normal
  WS2812FX API, where each virtual strip can have its own brightness, effect,
  speed, segments, etc. The physical strip is used to drive the LLEDs. Behind
  the scenes WS2812FXT takes care of blending the two virtual strips together
  to create the transition from v1 to v2 on the physical strip.

  To transition from the v1 effect to the v2 effect, call the startTransition()
  function passing in the duration of the transition (in ms), and optionally
  the transition direction (i.e direction == true produces a transition from
  v1 to v2, while direction == false produces a transition from v2 to v1).

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2021  Keith Lord 

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
  2021-08-20 initial version
*/

#include <WS2812FX.h>

#define LED_PIN     4
#define LED_COUNT 144

WS2812FXT ws2812fxt = WS2812FXT(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {

  ws2812fxt.init();

  // setup the effects on the two virtual strips
  // note v1 and v2 are pointers, so you must use pointer syntax
  // (i.e. use ws2812fxt.v1->setMode(...), not ws2812fxt.v1.setMode(...))
  ws2812fxt.v1->setBrightness(32);
  ws2812fxt.v1->setSegment(0, 0, LED_COUNT-1, FX_MODE_RAINBOW_CYCLE, BLACK, 5000);

  ws2812fxt.v2->setBrightness(128);
  ws2812fxt.v2->setSegment(0, 0, LED_COUNT-1, FX_MODE_LARSON_SCANNER, BLUE, 3000);

  ws2812fxt.start();
}

void loop() {
  ws2812fxt.service();

  // every ten seconds transition from v1 to v2 then back to v1
  static bool direction = true;
  static unsigned long timer = 10000;
  unsigned long now = millis();

  if(now > timer) {
    ws2812fxt.startTransition(5000, direction); // transition duration = 5 seconds
    direction = !direction; // change the transition direction
    timer = now + 10000; // transition every ten seconds
  }
}
