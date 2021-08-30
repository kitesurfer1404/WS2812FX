# WS2812FX API

The WS2812FX Users Guide documents the major library functions used for creating
a lighting sketch, but the library also contains many "helper" functions
that may be of use to more advanced developers.
This document will list and elaborate on the libraries API.

***

## Disclaimer

Although the voltages and components used in WS2812FX projects are generally
safe, working with electronics does carry some risk. So…

Descriptions of circuits, software and other related information in this
document are provided only to illustrate the use of WS2812FX. You are fully
responsible for the incorporation of these circuits, software, and information
in the design of your project. WS2812FX contributors and maintainers assume no
responsibility for any losses incurred by you or third parties arising from the
use of these circuits, software, or information.

The authors have used reasonable care in preparing the information included in
this document, but we do not warrant that such information is error free. We
assume no liability whatsoever for any damages resulting from errors in or
omissions from the information included herein.

---
## Getting Feedback About How an Animation is Progressing

The isFrame() and isCycle() can be used to provide feedback about the state
of an effect to the user's sketch.

*isFrame()* returns true when the LEDs in a segment	have changed (think of it
as marking one "frame" of animation).

*isCycle()* returns true when a repetitive effect ends a cycle. For instance,
the Larson Scanner effect will cause isCycle() to return true when the lights
have completed a cycle (i.e. lights traveling from the start of the segment
to the end and back to the start).  

These functions can be called in the loop() function, after the call to the
WS2812FX service(), to count animation frames or effect cycles, and synchronize
the lights to other events.
```c++
void loop() {
  ws2812fx.service();

  if(ws2812fx.isFrame()) {
    Serial.println("Frame complete");
  }
  if(ws2812fx.isCycle()) {
    Serial.println("Cycle complete");
  }
}
```
The ws2812fx_soundfx example sketch makes good use of this feature.

---
## Overriding the Default Number of Segments
By default WS2812FX creates data structures to support up to ten segments per
LED strip. Unfortunately, the data structures consume a small amount of SRAM
memory wether you use all ten segments or not. If you're programming a
memory constrained microprocessor, like an Arduino, you might want to reduce
the segment data structures to free up more memory for your sketch. Conversely,
if you want to partition your strip into more than ten segments, you can
increase the allowed number of segments per strip. You change the maximum number
of segments per strip by using an enhanced constructor that has two additional
parameters:
>WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, NUM_SEGMENTS, NUM_ACTIVE_SEGMENTS);

The *NUM_SEGMENTS* parameter sets the maximum number of segments allowed per
strip and *NUM_ACTIVE_SEGMENTS* sets how many segments can be active at
the same time.
```c++
// Setup a strip that can have only one segment (conserves memory)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, 1, 1);

// Setup a strip that can have up to 20 segments and all 20 can be active
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, 20, 20);

// Setup a strip that can have up to 20 segments, but only 5 can be active at
// the same time. This scenario might be used if you're creating 20 idle
// (inactive) segments, and dynamically activating a subset segments (5 or
// fewer) based on external events. See the "Segment management" section of
// the WS2812FX users guide.
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800, 20, 5);
```

---
## Active and Idle Segments
When you want to create dynamic lighting, that is, lighting that changes over
time, typically you would simply run setSegment statements to update the LEDs
based on some external event. Like so:
```c++
unsigned long timer = 0; // a timer
void loop() {
  unsigned long now = millis();
  ws2812fx.service();

  if((now - timer) > 10000) { // every 10 seconds
    uint32_t newColor = ws2812fx.color_wheel(ws2812fx.random8()); // a random color
    ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_STATIC, newColor, 1000);
    timer = now; // reset the timer
  }
}
```

Alternatively, you can setup active and idle segments at the beginning of the
sketch and switch which segments are active based on some external event.
```c++
void setup() {
...
  // create one active segment and one idle segment
  // note both segments span the entire LED strip
  ws2812fx.setSegment    (0, 0, LED_COUNT-1, FX_MODE_BLINK, YELLOW, 2000); // seg[0]
  ws2812fx.setIdleSegment(1, 0, LED_COUNT-1, FX_MODE_BLINK, GREEN,  2000); // seg[1]
...
}

unsigned long timer = 0; // a timer
void loop() {
  unsigned long now = millis();
  ws2812fx.service();

  if((now - timer) > 10000) { // every 10 seconds
    if(ws2812fx.isActiveSegment(0)) {   // if seg[0] is active, switch to seg[1]
      ws2812fx.swapActiveSegment(0, 1);
    } else {                            // else, switch to seg[0]
      ws2812fx.swapActiveSegment(1, 0);
    }
    timer = now; // reset the timer
  }
}
```
The library provides these functions to manage active/idle segments:  
  - addActiveSegment(seg) - makes a segment active
  - removeActiveSegment(seg) - makes a segment idle
  - swapActiveSegment(oldSeg, newSeg) - makes segment *oldSeg* idle, and segment *newSeg* active
  - isActiveSegment(seg) - returns true if a segment is active
  - getActiveSegments() - returns an array of _NUM_ACTIVE_SEGMENTS_ bytes representing the active/idle state of each segment. A byte value of 255 indicates the segment is idle , and any other value indicates the segment is active.

The *ws2812fx_segment_sequence* example sketch demonstrates this technique.

---
## Miscellaneous Helper Functions
  **Fast random number generation**
  - random8() - return an 8-bit random number
  - random8(lim) - return an 8-bit random number between 0 and (lim - 1)
  - random16() - return a 16-bit random number
  - random16(lim) - return a 16-bit random number between 0 and (lim - 1)
  - setRandomSeed(seed) - set the 16-bit random number generator seed
  - get_random_wheel_index(num) - returns an 8-bit random number that is at least 42 more or less than _num_. Used mostly when generating a new random color that is significantly different from the current color:  
  ```c++
  // generate a new, significantly different, random color
  uint8_t newColorIndex = ws2812fx.get_random_wheel_index(oldColorIndex);
  uint32_t newColor = ws2812fx.color_wheel(newColorIndex);
  ```

  **Color functions**
  - getColor() - returns the 32-bit color for segment 0.
  - getColor(seg) - returns the 32-bit color for segment _seg_.
  - getColors(seg) - returns an array of three 32-bit colors for segment _seg_.
  - color_wheel(index) - return a 32-bit RGB color value. The 8-bit index parameter selects one color from a virtual table of 256 colors. The virtual table contains a rainbow of colors in ROYGBIV order:  
  red - orange - yellow - green - blue -indigo - violet - red
  ```c++
  // generate a random color
  uint32_t color = ws2812fx.color_wheel(ws2812fx.random8());
  ```
  - color_blend(color1, color2, blendAmount) - returns the resulting 32-bit color created by blending together _color1_ and _color2_ by _blendAmount_. If _blendAmount_ = 0, color1 is returned. If _blendAmount_ = 255, color2 is returned. For intermediate values of _blendAmount_, a proportionally blended color is returned.

  - intensitySum() - returns the 32-bit sum of all LED intensities. Used for making strip power estimates.
  - intensitySums() - returns an array of three (for RGB LEDs) or four (for RGBW LEDs) 32-bit values. Each value is the sum of all individual RGB color intensities. Used for making strip power estimates. Slower than _intensitySum()_, but gives more fine grained detail for each color's power estimate.

---
**LED strip functions**
- setPixels(numPixels, ptr) - frees the underlaying Adafruit_Neopixel pixels array, and swaps it out for a user created pixels array. Used carefully, it can allow greater access to the raw pixel data. The _ws2812fx_virtual_strip_ example sketch uses this technique to drive two physical strips with one virtual strip.
```c++
uint8_t myPixels[LED_COUNT * 3]; // 3 bytes per LED for RGB LEDs
setPixels(LED_COUNT, myPixels);
```
- getNumBytes() - returns the number of bytes allocated to the pixels array.
- getNumBytesPerPixel() - returns 3 if the strip is RGB LEDs, or 4 if the strip is RGBW LEDs
- blend() - blends pixel data from two strips.
```c++
// blend the pixel data from strips ws2812fx1 and ws2812fx2
// to create the pixel data for strip ws2812fx3
uint8_t *src1_p = ws2812fx1.getPixels(); // get pointers to the strips' pixel data
uint8_t *src2_p = ws2812fx2.getPixels();
uint8_t *dest_p = ws2812fx3.getPixels();
uint16_t numBytes = ws2812fx3.getNumBytes(); // number of bytes of pixel data
uint8_t blendAmt = 128; // blend amount 0-255: 0 = all ws2812fx1, 255 = all ws2812fx2
ws2812fx3.blend(dest_p, src1_p, src2_p, numBytes, blendAmt);
```



---
To be continued...
