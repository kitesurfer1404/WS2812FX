/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit NeoPixel Library

  NOTES
    * Uses the Adafruit NeoPixel library. Get it here:
      https://github.com/adafruit/Adafruit_NeoPixel



  LICENSE

  The MIT License (MIT)

  Copyright (c) 2016  Harm Aldick

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

  2016-05-28   Initial beta release
  2016-06-03   Code cleanup, minor improvements, new modes
  2016-06-04   2 new fx, fixed setColor (now also resets _mode_color)
  2017-02-02   added external trigger functionality (e.g. for sound-to-light)
  2017-02-02   removed "blackout" on mode, speed or color-change
  2017-09-26   implemented segment and reverse features
  2017-11-16   changed speed calc, reduced memory footprint
  2018-02-24   added hooks for user created custom effects
*/

#include "WS2812FX.h"

void WS2812FX::init() {
  resetSegmentRuntimes();
  Adafruit_NeoPixel::begin();
}

// void WS2812FX::timer() {
//     for (int j=0; j < 1000; j++) {
//       uint16_t delay = (this->*_mode[SEGMENT.mode])();
//     }
// }

void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    bool doShow = false;
    for(uint8_t i=0; i < _num_segments; i++) {
      _segment_index = i;
      CLR_FRAME;
      if(now > SEGMENT_RUNTIME.next_time || _triggered) {
        SET_FRAME;
        doShow = true;
        uint16_t delay = (this->*_mode[SEGMENT.mode])();
        SEGMENT_RUNTIME.next_time = now + max(delay, SPEED_MIN);
        SEGMENT_RUNTIME.counter_mode_call++;
      }
    }
    if(doShow) {
      delay(1); // for ESP32 (see https://forums.adafruit.com/viewtopic.php?f=47&t=117327)
      show();
    }
    _triggered = false;
  }
}

// overload setPixelColor() functions so we can use gamma correction
// (see https://learn.adafruit.com/led-tricks-gamma-correction/the-issue)
void WS2812FX::setPixelColor(uint16_t n, uint32_t c) {
  if(IS_GAMMA) {
    uint8_t w = (c >> 24) & 0xFF;
    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >>  8) & 0xFF;
    uint8_t b =  c        & 0xFF;
    Adafruit_NeoPixel::setPixelColor(n, gamma8(r), gamma8(g), gamma8(b), gamma8(w));
  } else {
    Adafruit_NeoPixel::setPixelColor(n, c);
  }
}

void WS2812FX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  if(IS_GAMMA) {
    Adafruit_NeoPixel::setPixelColor(n, gamma8(r), gamma8(g), gamma8(b));
  } else {
    Adafruit_NeoPixel::setPixelColor(n, r, g, b);
  }
}

void WS2812FX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  if(IS_GAMMA) {
    Adafruit_NeoPixel::setPixelColor(n, gamma8(r), gamma8(g), gamma8(b), gamma8(w));
  } else {
    Adafruit_NeoPixel::setPixelColor(n, r, g, b, w);
  }
}

// overload show() functions so we can use custom show()
void WS2812FX::show(void) {
  if(customShow == NULL) {
    Adafruit_NeoPixel::show();
  } else {
    customShow();
  }
}

void WS2812FX::start() {
  resetSegmentRuntimes();
  _running = true;
}

void WS2812FX::stop() {
  _running = false;
  strip_off();
}

void WS2812FX::pause() {
  _running = false;
}

void WS2812FX::resume() {
  _running = true;
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::setMode(uint8_t m) {
  setMode(0, m);
}

void WS2812FX::setMode(uint8_t seg, uint8_t m) {
  resetSegmentRuntime(seg);
  _segments[seg].mode = constrain(m, 0, MODE_COUNT - 1);
}

void WS2812FX::setOptions(uint8_t seg, uint8_t o) {
  _segments[seg].options = o;
}

void WS2812FX::setSpeed(uint16_t s) {
  setSpeed(0, s);
}

void WS2812FX::setSpeed(uint8_t seg, uint16_t s) {
  resetSegmentRuntime(seg);
  _segments[seg].speed = constrain(s, SPEED_MIN, SPEED_MAX);
}

void WS2812FX::increaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.speed + s, SPEED_MIN, SPEED_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(SEGMENT.speed - s, SPEED_MIN, SPEED_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint32_t c) {
  setColor(0, c);
}

void WS2812FX::setColor(uint8_t seg, uint32_t c) {
  resetSegmentRuntime(seg);
  _segments[seg].colors[0] = c;
}

void WS2812FX::setColors(uint8_t seg, uint32_t* c) {
  resetSegmentRuntime(seg);
  for(uint8_t i=0; i<NUM_COLORS; i++) {
    _segments[seg].colors[i] = c[i];
  }
}

void WS2812FX::setBrightness(uint8_t b) {
  b = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  Adafruit_NeoPixel::setBrightness(b);
  show();
}

void WS2812FX::increaseBrightness(uint8_t s) {
  s = constrain(getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(uint8_t s) {
  s = constrain(getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::setLength(uint16_t b) {
  resetSegmentRuntimes();
  if (b < 1) b = 1;

  // Decrease numLEDs to maximum available memory
  do {
      Adafruit_NeoPixel::updateLength(b);
      b--;
  } while(!Adafruit_NeoPixel::numLEDs && b > 1);

  _segments[0].start = 0;
  _segments[0].stop = Adafruit_NeoPixel::numLEDs - 1;
}

void WS2812FX::increaseLength(uint16_t s) {
  s = _segments[0].stop - _segments[0].start + 1 + s;
  setLength(s);
}

void WS2812FX::decreaseLength(uint16_t s) {
  if (s > _segments[0].stop - _segments[0].start + 1) s = 1;
  s = _segments[0].stop - _segments[0].start + 1 - s;

  for(uint16_t i=_segments[0].start + s; i <= (_segments[0].stop - _segments[0].start + 1); i++) {
    setPixelColor(i, 0);
  }
  show();

  setLength(s);
}

boolean WS2812FX::isRunning() {
  return _running;
}

boolean WS2812FX::isTriggered() {
  return _triggered;
}

boolean WS2812FX::isFrame() {
  return isFrame(0);
}

boolean WS2812FX::isFrame(uint8_t segIndex) {
  return (_segment_runtimes[segIndex].aux_param2 & FRAME);
}

boolean WS2812FX::isCycle() {
  return isCycle(0);
}

boolean WS2812FX::isCycle(uint8_t segIndex) {
  return (_segment_runtimes[segIndex].aux_param2 & CYCLE);
}

uint8_t WS2812FX::getMode(void) {
  return getMode(0);
}

uint8_t WS2812FX::getMode(uint8_t seg) {
  return _segments[seg].mode;
}

uint16_t WS2812FX::getSpeed(void) {
  return getSpeed(0);
}

uint16_t WS2812FX::getSpeed(uint8_t seg) {
  return _segments[seg].speed;
}


uint8_t WS2812FX::getOptions(uint8_t seg) {
  return _segments[seg].options;
}

uint16_t WS2812FX::getLength(void) {
  return numPixels();
}

uint8_t WS2812FX::getModeCount(void) {
  return MODE_COUNT;
}

uint8_t WS2812FX::getNumSegments(void) {
  return _num_segments;
}

void WS2812FX::setNumSegments(uint8_t n) {
  _num_segments = n;
}

uint32_t WS2812FX::getColor(void) {
  return getColor(0);
}

uint32_t WS2812FX::getColor(uint8_t seg) {
  return _segments[seg].colors[0];
}

uint32_t* WS2812FX::getColors(uint8_t seg) {
  return _segments[seg].colors;
}

WS2812FX::Segment* WS2812FX::getSegment(void) {
  return &_segments[_segment_index];
}

WS2812FX::Segment* WS2812FX::getSegment(uint8_t seg) {
  return &_segments[seg];
}

WS2812FX::Segment* WS2812FX::getSegments(void) {
  return _segments;
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntime(void) {
  return &_segment_runtimes[_segment_index];
}

void WS2812FX::setSegmentRuntime(WS2812FX::Segment_runtime* segment_runtime) {
  _segment_runtimes[_segment_index] = *segment_runtime;
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntime(uint8_t seg) {
  return &_segment_runtimes[seg];
}

void WS2812FX::setSegmentRuntime(WS2812FX::Segment_runtime* segment_runtime, uint8_t seg) {
  _segment_runtimes[seg] = *segment_runtime;
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntimes(void) {
  return _segment_runtimes;
}

void WS2812FX::setSegmentRuntimes(WS2812FX::Segment_runtime* segment_runtimes) {
  memcpy(_segment_runtimes, segment_runtimes, MAX_NUM_SEGMENTS);
}


const __FlashStringHelper* WS2812FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _names[m];
  } else {
    return F("");
  }
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse) {
  uint32_t colors[] = {color, 0, 0};
  setSegment(n, start, stop, mode, colors, speed, reverse);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse) {
  setSegment(n, start, stop, mode, colors, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options) {
  if(n < (sizeof(_segments) / sizeof(_segments[0]))) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].options = options;

    for(uint8_t i=0; i<NUM_COLORS; i++) {
      _segments[n].colors[i] = colors[i];
    }
  }
}

void WS2812FX::resetSegments() {
  resetSegmentRuntimes();
  memset(_segments, 0, sizeof(_segments));
  _segment_index = 0;
  _num_segments = 1;
  setSegment(0, 0, 7, FX_MODE_STATIC, (const uint32_t[]){DEFAULT_COLOR, 0, 0}, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::resetSegmentRuntimes() {
  memset(_segment_runtimes, 0, sizeof(_segment_runtimes));
}

void WS2812FX::resetSegmentRuntime(uint8_t seg) {
  memset(&_segment_runtimes[seg], 0, sizeof(_segment_runtimes[0]));
}

/* #####################################################
#
#  Color and Blinken Functions
#
##################################################### */

/*
 * Turns everything off. Doh.
 */
void WS2812FX::strip_off() {
  Adafruit_NeoPixel::clear();
  show();
}


/*
 * Put a value 0 to 255 in to get a color value.
 * The colours are a transition r -> g -> b -> back to r
 * Inspired by the Adafruit examples.
 */
uint32_t WS2812FX::color_wheel(uint8_t pos) {
  pos = 255 - pos;
  if(pos < 85) {
    return ((uint32_t)(255 - pos * 3) << 16) | ((uint32_t)(0) << 8) | (pos * 3);
  } else if(pos < 170) {
    pos -= 85;
    return ((uint32_t)(0) << 16) | ((uint32_t)(pos * 3) << 8) | (255 - pos * 3);
  } else {
    pos -= 170;
    return ((uint32_t)(pos * 3) << 16) | ((uint32_t)(255 - pos * 3) << 8) | (0);
  }
}


/*
 * Returns a new, random wheel index with a minimum distance of 42 from pos.
 */
uint8_t WS2812FX::get_random_wheel_index(uint8_t pos) {
  uint8_t r = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t d = 0;

  while(d < 42) {
    r = random8();
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }

  return r;
}

// fast 8-bit random number generator shamelessly borrowed from FastLED
uint8_t WS2812FX::random8() {
    _rand16seed = (_rand16seed * 2053) + 13849;
    return (uint8_t)((_rand16seed + (_rand16seed >> 8)) & 0xFF);
}

// note random8(uint8_t) generates numbers in the range 0 - 254, 255 is never generated
uint8_t WS2812FX::random8(uint8_t lim) {
    uint8_t r = random8();
    r = (r * lim) >> 8;
    return r;
}

/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }
  return 500;
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe) {
  uint32_t color = ((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) ? color1 : color2;
  if(IS_REVERSE) color = (color == color1) ? color2 : color1;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  if((SEGMENT_RUNTIME.counter_mode_call & 1) == 0) {
    return strobe ? 20 : (SEGMENT.speed / 2);
  } else {
    return strobe ? SEGMENT.speed - 20 : (SEGMENT.speed / 2);
  }
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], false);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
  return blink(SEGMENT.colors[0], SEGMENT.colors[1], true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
  return blink(color_wheel(SEGMENT_RUNTIME.counter_mode_call & 0xFF), SEGMENT.colors[1], true);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2, bool rev) {
  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.stop - led_offset, color1);
    } else {
      setPixelColor(SEGMENT.start + led_offset, color1);
    }
  } else {
    uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    if((IS_REVERSE && !rev) || (!IS_REVERSE && rev)) {
      setPixelColor(SEGMENT.stop - led_offset, color2);
    } else {
      setPixelColor(SEGMENT.start + led_offset, color2);
    }
  }

  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) SET_CYCLE;
  else CLR_CYCLE;

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 2);
  return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
}

/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], false);
}

uint16_t WS2812FX::mode_color_wipe_inv(void) {
  return color_wipe(SEGMENT.colors[1], SEGMENT.colors[0], false);
}

uint16_t WS2812FX::mode_color_wipe_rev(void) {
  return color_wipe(SEGMENT.colors[0], SEGMENT.colors[1], true);
}

uint16_t WS2812FX::mode_color_wipe_rev_inv(void) {
  return color_wipe(SEGMENT.colors[1], SEGMENT.colors[0], true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, false) * 2;
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);
  return color_wipe(color, color, true) * 2;
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param); // aux_param will store our random color wheel index
  uint32_t color = color_wheel(SEGMENT_RUNTIME.aux_param);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }
  return (SEGMENT.speed);
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 */
uint16_t WS2812FX::mode_single_dynamic(void) {
  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color_wheel(random8()));
    }
  }

  setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color_wheel(random8()));
  return (SEGMENT.speed);
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color_wheel(random8()));
  }
  return (SEGMENT.speed);
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 15 -> 255 -> 15

  uint16_t delay;
  if(lum == 15) delay = 970; // 970 pause before each breath
  else if(lum <=  25) delay = 38; // 19
  else if(lum <=  50) delay = 36; // 18
  else if(lum <=  75) delay = 28; // 14
  else if(lum <= 100) delay = 20; // 10
  else if(lum <= 125) delay = 14; // 7
  else if(lum <= 150) delay = 11; // 5
  else delay = 10; // 4

  uint32_t color = SEGMENT.colors[0];
  uint8_t w = (color >> 24 & 0xFF) * lum / 256;
  uint8_t r = (color >> 16 & 0xFF) * lum / 256;
  uint8_t g = (color >>  8 & 0xFF) * lum / 256;
  uint8_t b = (color       & 0xFF) * lum / 256;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, r, g, b, w);
  }

  SEGMENT_RUNTIME.counter_mode_step += 2;
  if(SEGMENT_RUNTIME.counter_mode_step > (512-15)) SEGMENT_RUNTIME.counter_mode_step = 15;
  return delay;
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void) {
  int lum = SEGMENT_RUNTIME.counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0

  uint32_t color = color_blend(SEGMENT.colors[0], SEGMENT.colors[1], lum);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step += 4;
  if(SEGMENT_RUNTIME.counter_mode_step > 511) SEGMENT_RUNTIME.counter_mode_step = 0;
  return (SEGMENT.speed / 128);
}


/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[1]);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset); 

  if(IS_REVERSE) {
    setPixelColor(SEGMENT.stop - led_offset, SEGMENT.colors[0]);
  } else {
    setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[1]);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset);

  setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + SEGMENT_LENGTH - led_offset - 1, SEGMENT.colors[0]);

  SEGMENT_RUNTIME.counter_mode_step++;
  return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(SEGMENT_RUNTIME.counter_mode_step);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return (SEGMENT.speed / 256);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
	  uint32_t color = color_wheel(((i * 256 / SEGMENT_LENGTH) + SEGMENT_RUNTIME.counter_mode_step) & 0xFF);
    setPixelColor(SEGMENT.start + i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return (SEGMENT.speed / 256);
}


/*
 * theater chase function
 */
uint16_t WS2812FX::theater_chase(uint32_t color1, uint32_t color2) {
  SEGMENT_RUNTIME.counter_mode_call = SEGMENT_RUNTIME.counter_mode_call % 3;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i % 3) == SEGMENT_RUNTIME.counter_mode_call) {
      if(IS_REVERSE) {
        setPixelColor(SEGMENT.stop - i, color1);
      } else {
        setPixelColor(SEGMENT.start + i, color1);
      }
    } else {
      if(IS_REVERSE) {
        setPixelColor(SEGMENT.stop - i, color2);
      } else {
        setPixelColor(SEGMENT.start + i, color2);
      }
    }
  }

  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return theater_chase(SEGMENT.colors[0], SEGMENT.colors[1]);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0xFF;
  return theater_chase(color_wheel(SEGMENT_RUNTIME.counter_mode_step), BLACK);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
  uint8_t w = ((SEGMENT.colors[0] >> 24) & 0xFF);
  uint8_t r = ((SEGMENT.colors[0] >> 16) & 0xFF);
  uint8_t g = ((SEGMENT.colors[0] >>  8) & 0xFF);
  uint8_t b =  (SEGMENT.colors[0]        & 0xFF);

  uint8_t sineIncr = max(1, (256 / SEGMENT_LENGTH));
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    int lum = (int)sine8(((i + SEGMENT_RUNTIME.counter_mode_step) * sineIncr));
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.start + i, (r * lum) / 256, (g * lum) / 256, (b * lum) / 256, (w * lum) / 256);
    } else {
      setPixelColor(SEGMENT.stop - i,  (r * lum) / 256, (g * lum) / 256, (b * lum) / 256, (w * lum) / 256);
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 256;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * twinkle function
 */
uint16_t WS2812FX::twinkle(uint32_t color1, uint32_t color2) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, color2);
    }
    uint16_t min_leds = max(1, SEGMENT_LENGTH / 5); // make sure, at least one LED is on
    uint16_t max_leds = max(1, SEGMENT_LENGTH / 2); // make sure, at least one LED is on
    SEGMENT_RUNTIME.counter_mode_step = random(min_leds, max_leds);
  }

  setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color1);

  SEGMENT_RUNTIME.counter_mode_step--;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void) {
  return twinkle(SEGMENT.colors[0], SEGMENT.colors[1]);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle_random(void) {
  return twinkle(color_wheel(random8()), SEGMENT.colors[1]);
}


/*
 * fade out function
 */
void WS2812FX::fade_out() {
  static const uint8_t rateMapH[] = {0, 1, 1, 1, 2, 3, 4, 6};
  static const uint8_t rateMapL[] = {0, 2, 3, 8, 8, 8, 8, 8};

  uint8_t rate  = FADE_RATE;
  uint8_t rateH = rateMapH[rate];
  uint8_t rateL = rateMapL[rate];

  uint32_t color = SEGMENT.colors[1]; // target color
  int w2 = (color >> 24) & 0xff;
  int r2 = (color >> 16) & 0xff;
  int g2 = (color >>  8) & 0xff;
  int b2 =  color        & 0xff;

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    color = getPixelColor(i);
    if(rate == 0) { // old fade-to-black algorithm
      setPixelColor(i, (color >> 1) & 0x7F7F7F7F);
    } else { // new fade-to-color algorithm
      int w1 = (color >> 24) & 0xff; // current color
      int r1 = (color >> 16) & 0xff;
      int g1 = (color >>  8) & 0xff;
      int b1 =  color        & 0xff;

      // calculate the color differences between the current and target colors
      int wdelta = w2 - w1;
      int rdelta = r2 - r1;
      int gdelta = g2 - g1;
      int bdelta = b2 - b1;

      // if the current and target colors are almost the same, jump right to the target
      // color, otherwise calculate an intermediate color. (fixes rounding issues)
      wdelta = abs(wdelta) < 3 ? wdelta : (wdelta >> rateH) + (wdelta >> rateL);
      rdelta = abs(rdelta) < 3 ? rdelta : (rdelta >> rateH) + (rdelta >> rateL);
      gdelta = abs(gdelta) < 3 ? gdelta : (gdelta >> rateH) + (gdelta >> rateL);
      bdelta = abs(bdelta) < 3 ? bdelta : (bdelta >> rateH) + (bdelta >> rateL);

      setPixelColor(i, r1 + rdelta, g1 + gdelta, b1 + bdelta, w1 + wdelta);
    }
  }
}


/*
 * color blend function
 */
uint32_t WS2812FX::color_blend(uint32_t color1, uint32_t color2, uint8_t blend) {
  if(blend == 0)   return color1;
  if(blend == 255) return color2;

  uint8_t w1 = (color1 >> 24) & 0xff;
  uint8_t r1 = (color1 >> 16) & 0xff;
  uint8_t g1 = (color1 >>  8) & 0xff;
  uint8_t b1 =  color1        & 0xff;

  uint8_t w2 = (color2 >> 24) & 0xff;
  uint8_t r2 = (color2 >> 16) & 0xff;
  uint8_t g2 = (color2 >>  8) & 0xff;
  uint8_t b2 =  color2        & 0xff;

  uint32_t w3 = ((w2 * blend) + (w1 * (255U - blend))) / 256U;
  uint32_t r3 = ((r2 * blend) + (r1 * (255U - blend))) / 256U;
  uint32_t g3 = ((g2 * blend) + (g1 * (255U - blend))) / 256U;
  uint32_t b3 = ((b2 * blend) + (b1 * (255U - blend))) / 256U;

  return ((w3 << 24) | (r3 << 16) | (g3 << 8) | (b3));
}

// Return the sum of all LED intensities (can be used for
// rudimentary power calculations)
uint32_t WS2812FX::intensitySum() {
  uint8_t *pixels = getPixels();
  uint32_t sum = 0;
  for(uint16_t i=0; i <numBytes; i++) {
    sum+= pixels[i];
  }
  return sum;
}

// Return the sum of each color's intensity. Note, the order of
// intensities in the returned array depends on the type of WS2812
// LEDs you have. NEO_GRB LEDs will return an array with entries
// in a different order then NEO_RGB LEDs.
uint32_t* WS2812FX::intensitySums() {
  static uint32_t intensities[] = { 0, 0, 0, 0 };
  memset(intensities, 0, sizeof(intensities));

  uint8_t *pixels = getPixels();
  uint8_t bytesPerPixel = (wOffset == rOffset) ? 3 : 4; // 3=RGB, 4=RGBW
  for(uint16_t i=0; i <numBytes; i += bytesPerPixel) {
    intensities[0] += pixels[i];
    intensities[1] += pixels[i + 1];
    intensities[2] += pixels[i + 2];
    if(bytesPerPixel == 4) intensities[3] += pixels[i + 3]; // for RGBW LEDs
  }
  return intensities;
}


/*
 * twinkle_fade function
 */
uint16_t WS2812FX::twinkle_fade(uint32_t color) {
  fade_out();

  if(random8(3) == 0) {
    setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
  }
  return (SEGMENT.speed / 8);
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  return twinkle_fade(SEGMENT.colors[0]);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade_random(void) {
  return twinkle_fade(color_wheel(random8()));
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void) {
  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param3, SEGMENT.colors[1]);
  SEGMENT_RUNTIME.aux_param3 = random(SEGMENT_LENGTH); // aux_param3 stores the random led index
  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param3, SEGMENT.colors[0]);
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * Lights all LEDs in the color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      setPixelColor(i, SEGMENT.colors[0]);
    }
  }

  setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param3, SEGMENT.colors[0]);

  if(random8(5) == 0) {
    SEGMENT_RUNTIME.aux_param3 = random(SEGMENT_LENGTH); // aux_param3 stores the random led index
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.aux_param3, WHITE);
    return 20;
  } 
  return SEGMENT.speed;
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }

  if(random8(5) < 2) {
    for(uint16_t i=0; i < max(1, SEGMENT_LENGTH/3); i++) {
      setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), WHITE);
    }
    return 20;
  }
  return SEGMENT.speed;
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, BLACK);
  }

  uint16_t delay = 200 + ((9 - (SEGMENT.speed % 10)) * 100);
  uint16_t count = 2 * ((SEGMENT.speed / 100) + 1);
  if(SEGMENT_RUNTIME.counter_mode_step < count) {
    if((SEGMENT_RUNTIME.counter_mode_step & 1) == 0) {
      for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
        setPixelColor(i, SEGMENT.colors[0]);
      }
      delay = 20;
    } else {
      delay = 50;
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (count + 1);
  return delay;
}


/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */

uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  uint16_t a = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t b = (a + 1) % SEGMENT_LENGTH;
  uint16_t c = (b + 1) % SEGMENT_LENGTH;
  if(IS_REVERSE) {
    setPixelColor(SEGMENT.stop - a, color1);
    setPixelColor(SEGMENT.stop - b, color2);
    setPixelColor(SEGMENT.stop - c, color3);
  } else {
    setPixelColor(SEGMENT.start + a, color1);
    setPixelColor(SEGMENT.start + b, color2);
    setPixelColor(SEGMENT.start + c, color3);
  }

  if(b == 0) SET_CYCLE;
  else CLR_CYCLE;

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * Bicolor chase mode
 */
uint16_t WS2812FX::mode_bicolor_chase(void) {
  return chase(SEGMENT.colors[0], SEGMENT.colors[1], SEGMENT.colors[2]);
}


/*
 * White running on _color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  return chase(SEGMENT.colors[0], WHITE, WHITE);
}


/*
 * Black running on _color.
 */
uint16_t WS2812FX::mode_chase_blackout(void) {
  return chase(SEGMENT.colors[0], BLACK, BLACK);
}


/*
 * _color running on white.
 */
uint16_t WS2812FX::mode_chase_white(void) {
  return chase(WHITE, SEGMENT.colors[0], SEGMENT.colors[0]);
}


/*
 * White running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }
  return chase(color_wheel(SEGMENT_RUNTIME.aux_param), WHITE, WHITE);
}


/*
 * Rainbow running on white.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  uint32_t color2 = color_wheel(((n * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);

  return chase(WHITE, color2, color3);
}


/*
 * White running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGMENT_LENGTH;
  uint8_t color_index = SEGMENT_RUNTIME.counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((SEGMENT_RUNTIME.counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, WHITE, WHITE);
}


/*
 * Black running on rainbow.
 */
uint16_t WS2812FX::mode_chase_blackout_rainbow(void) {
  uint8_t color_sep = 256 / SEGMENT_LENGTH;
  uint8_t color_index = SEGMENT_RUNTIME.counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((SEGMENT_RUNTIME.counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, BLACK, BLACK);
}


/*
 * White flashes running on _color.
 */
uint16_t WS2812FX::mode_chase_flash(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    setPixelColor(i, SEGMENT.colors[0]);
  }

  uint16_t delay = (SEGMENT.speed / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
      uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
      if(IS_REVERSE) {
        setPixelColor(SEGMENT.stop - n, WHITE);
        setPixelColor(SEGMENT.stop - m, WHITE);
      } else {
        setPixelColor(SEGMENT.start + n, WHITE);
        setPixelColor(SEGMENT.start + m, WHITE);
      }
      delay = 20;
    } else {
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  }
  return delay;
}


/*
 * White flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=0; i < SEGMENT_RUNTIME.counter_mode_step; i++) {
    setPixelColor(SEGMENT.start + i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  uint16_t delay = (SEGMENT.speed / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
    uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
    if(flash_step % 2 == 0) {
      setPixelColor(SEGMENT.start + n, WHITE);
      setPixelColor(SEGMENT.start + m, WHITE);
      delay = 20;
    } else {
      setPixelColor(SEGMENT.start + n, color_wheel(SEGMENT_RUNTIME.aux_param));
      setPixelColor(SEGMENT.start + m, BLACK);
      delay = 30;
    }
  } else {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;

    if(SEGMENT_RUNTIME.counter_mode_step == 0) {
      SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    }
  }
  return delay;
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i + SEGMENT_RUNTIME.counter_mode_step) % 4 < 2) {
      if(IS_REVERSE) {
        setPixelColor(SEGMENT.start + i, color1);
      } else {
        setPixelColor(SEGMENT.stop - i, color1);
      }
    } else {
      if(IS_REVERSE) {
        setPixelColor(SEGMENT.start + i, color2);
      } else {
        setPixelColor(SEGMENT.stop - i, color2);
      }
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) & 0x3;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(SEGMENT.colors[0], WHITE);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX::mode_running_red_blue(void) {
  return running(RED, BLUE);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX::mode_merry_christmas(void) {
  return running(RED, GREEN);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX::mode_halloween(void) {
  return running(PURPLE, ORANGE);
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX::mode_running_random(void) {
  for(uint16_t i=SEGMENT_LENGTH-1; i > 0; i--) {
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.stop - i, Adafruit_NeoPixel::getPixelColor(SEGMENT.stop - i + 1));
    } else {
      setPixelColor(SEGMENT.start + i, Adafruit_NeoPixel::getPixelColor(SEGMENT.start + i - 1));
    }
  }

  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.stop, color_wheel(SEGMENT_RUNTIME.aux_param));
    } else {
      setPixelColor(SEGMENT.start, color_wheel(SEGMENT_RUNTIME.aux_param));
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step == 0) ? 1 : 0;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  fade_out();

  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    }
  } else {
    if(IS_REVERSE) {
      setPixelColor(SEGMENT.stop - ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) + 2, SEGMENT.colors[0]);
    } else {
      setPixelColor(SEGMENT.start + ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) - 2, SEGMENT.colors[0]);
    }
  }

  if(SEGMENT_RUNTIME.counter_mode_step % SEGMENT_LENGTH == 0) SET_CYCLE;
  else CLR_CYCLE;

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % ((SEGMENT_LENGTH * 2) - 2);
  return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  fade_out();

  if(IS_REVERSE) {
    setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  } else {
    setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::fireworks(uint32_t color) {
  fade_out();

//// set brightness(i) = brightness(i-1)/4 + brightness(i) + brightness(i+1)/4
/*
// the old way, so many calls to the pokey getPixelColor() function made this super slow
  for(uint16_t i=SEGMENT.start + 1; i <SEGMENT.stop; i++) {
    uint32_t prevLed = (Adafruit_NeoPixel::getPixelColor(i-1) >> 2) & 0x3F3F3F3F;
    uint32_t thisLed = Adafruit_NeoPixel::getPixelColor(i);
    uint32_t nextLed = (Adafruit_NeoPixel::getPixelColor(i+1) >> 2) & 0x3F3F3F3F;
    setPixelColor(i, prevLed + thisLed + nextLed);
  }
*/

// the new way, manipulate the Adafruit_NeoPixels pixels[] array directly, about 5x faster
  uint8_t *pixels = getPixels();
  uint8_t pixelsPerLed = (wOffset == rOffset) ? 3 : 4; // RGB or RGBW device
  uint16_t startPixel = SEGMENT.start * pixelsPerLed + pixelsPerLed;
  uint16_t stopPixel = SEGMENT.stop * pixelsPerLed ;
  for(uint16_t i=startPixel; i <stopPixel; i++) {
    uint16_t tmpPixel = (pixels[i - pixelsPerLed] >> 2) +
      pixels[i] +
      (pixels[i + pixelsPerLed] >> 2);
    pixels[i] =  tmpPixel > 255 ? 255 : tmpPixel;
  }

  if(!_triggered) {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/20); i++) {
      if(random8(10) == 0) {
        setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
      }
    }
  } else {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/10); i++) {
      setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
    }
  }
  return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Firework sparks.
 */
uint16_t WS2812FX::mode_fireworks(void) {
  return fireworks(SEGMENT.colors[0]);
}

/*
 * Random colored firework sparks.
 */
uint16_t WS2812FX::mode_fireworks_random(void) {
  return fireworks(color_wheel(random8()));
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity) {
  byte w = (SEGMENT.colors[0] >> 24) & 0xFF;
  byte r = (SEGMENT.colors[0] >> 16) & 0xFF;
  byte g = (SEGMENT.colors[0] >>  8) & 0xFF;
  byte b = (SEGMENT.colors[0]        & 0xFF);
  byte lum = max(w, max(r, max(g, b))) / rev_intensity;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    int flicker = random8(lum);
    setPixelColor(i, max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0));
  }
  return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Random flickering.
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  return fire_flicker(3);
}

/*
* Random flickering, less intensity.
*/
uint16_t WS2812FX::mode_fire_flicker_soft(void) {
  return fire_flicker(6);
}

/*
* Random flickering, more intensity.
*/
uint16_t WS2812FX::mode_fire_flicker_intense(void) {
  return fire_flicker(1.7);
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  uint16_t index = SEGMENT_RUNTIME.counter_mode_step % 6;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++, index++) {
    if(index > 5) index = 0;

    uint32_t color = color3;
    if(index < 2) color = color1;
    else if(index < 4) color = color2;

    if(IS_REVERSE) {
      setPixelColor(SEGMENT.start + i, color);
    } else {
      setPixelColor(SEGMENT.stop - i, color);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  return (SEGMENT.speed / SEGMENT_LENGTH);
}


/*
 * Tricolor chase mode
 */
uint16_t WS2812FX::mode_tricolor_chase(void) {
  return tricolor_chase(SEGMENT.colors[0], SEGMENT.colors[1], SEGMENT.colors[2]);
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX::mode_circus_combustus(void) {
  return tricolor_chase(RED, WHITE, BLACK);
}

/*
 * ICU mode
 */
uint16_t WS2812FX::mode_icu(void) {
  uint16_t dest = SEGMENT_RUNTIME.counter_mode_step & 0xFFFF;
 
  setPixelColor(SEGMENT.start + dest, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, SEGMENT.colors[0]);

  if(SEGMENT_RUNTIME.aux_param3 == dest) { // pause between eye movements
    if(random8(6) == 0) { // blink once in a while
      setPixelColor(SEGMENT.start + dest, BLACK);
      setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, BLACK);
      return 200;
    }
    SEGMENT_RUNTIME.aux_param3 = random(SEGMENT_LENGTH/2);
    return 1000 + random(2000);
  }

  setPixelColor(SEGMENT.start + dest, BLACK);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, BLACK);

  if(SEGMENT_RUNTIME.aux_param3 > SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step++;
    dest++;
  } else if (SEGMENT_RUNTIME.aux_param3 < SEGMENT_RUNTIME.counter_mode_step) {
    SEGMENT_RUNTIME.counter_mode_step--;
    dest--;
  }

  setPixelColor(SEGMENT.start + dest, SEGMENT.colors[0]);
  setPixelColor(SEGMENT.start + dest + SEGMENT_LENGTH/2, SEGMENT.colors[0]);

  return (SEGMENT.speed / SEGMENT_LENGTH);
}

/*
 * Custom modes
 */
uint16_t WS2812FX::mode_custom_0() {
  return customMode0();
}
uint16_t WS2812FX::mode_custom_1() {
  return customMode1();
}
uint16_t WS2812FX::mode_custom_2() {
  return customMode2();
}
uint16_t WS2812FX::mode_custom_3() {
  return customMode3();
}

/*
 * Custom mode helpers
 */
void WS2812FX::setCustomMode(uint16_t (*p)()) {
  customMode0 = p;
}

uint8_t WS2812FX::setCustomMode(const __FlashStringHelper* name, uint16_t (*p)()) {
  if(_custom_mode_index < MODE_COUNT) {
    _names[_custom_mode_index] = name; // store the custom mode name
    if(_custom_mode_index == FX_MODE_CUSTOM_0) customMode0 = p; // store the custom mode
    if(_custom_mode_index == FX_MODE_CUSTOM_1) customMode1 = p;
    if(_custom_mode_index == FX_MODE_CUSTOM_2) customMode2 = p;
    if(_custom_mode_index == FX_MODE_CUSTOM_3) customMode3 = p;

    _custom_mode_index++;
    return (_custom_mode_index - 1);
  }
  return 0;
}

/*
 * Custom show helper
 */
void WS2812FX::setCustomShow(void (*p)()) {
  customShow = p;
}
