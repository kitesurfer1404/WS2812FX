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
//       uint16_t delay = (this->*_modes[_seg->mode])();
//     }
// }

bool WS2812FX::service() {
  bool doShow = false;
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    for(uint8_t i=0; i < _active_segments_len; i++) {
      if(_active_segments[i] != INACTIVE_SEGMENT) {
        _seg     = &_segments[_active_segments[i]];
        _seg_len = (uint16_t)(_seg->stop - _seg->start + 1);
        _seg_rt  = &_segment_runtimes[i];
        CLR_FRAME_CYCLE;
        if(now > _seg_rt->next_time || _triggered) {
          SET_FRAME;
          doShow = true;
          uint16_t delay = (this->*_modes[_seg->mode])();
          _seg_rt->next_time = now + max(delay, SPEED_MIN);
          _seg_rt->counter_mode_call++;
        }
      }
    }
    if(doShow) {
      delay(1); // for ESP32 (see https://forums.adafruit.com/viewtopic.php?f=47&t=117327)
      show();
    }
    _triggered = false;
  }
  return doShow;
}

// overload setPixelColor() functions so we can use gamma correction
// (see https://learn.adafruit.com/led-tricks-gamma-correction/the-issue)
void WS2812FX::setPixelColor(uint16_t n, uint32_t c) {
  uint8_t w = (c >> 24) & 0xFF;
  uint8_t r = (c >> 16) & 0xFF;
  uint8_t g = (c >>  8) & 0xFF;
  uint8_t b =  c        & 0xFF;
  setPixelColor(n, r, g, b, w);
}

void WS2812FX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  setPixelColor(n, r, g, b, 0);
}

void WS2812FX::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  if(IS_GAMMA) {
    Adafruit_NeoPixel::setPixelColor(n, gamma8(r), gamma8(g), gamma8(b), gamma8(w));
  } else {
    Adafruit_NeoPixel::setPixelColor(n, r, g, b, w);
  }
}

void WS2812FX::copyPixels(uint16_t dest, uint16_t src, uint16_t count) {
  uint8_t *pixels = getPixels();
  uint8_t bytesPerPixel = getNumBytesPerPixel(); // 3=RGB, 4=RGBW

  memmove(pixels + (dest * bytesPerPixel), pixels + (src * bytesPerPixel), count * bytesPerPixel);
}

// change the underlying Adafruit_NeoPixel pixels pointer (use with care)
void WS2812FX::setPixels(uint16_t num_leds, uint8_t* ptr) {
  free(Adafruit_NeoPixel::pixels); // free existing data (if any)
  Adafruit_NeoPixel::pixels = ptr;
  Adafruit_NeoPixel::numLEDs = num_leds;
  Adafruit_NeoPixel::numBytes = num_leds * ((wOffset == rOffset) ? 3 : 4);
}

// overload show() functions so we can use custom show()
void WS2812FX::show(void) {
  customShow == NULL ? Adafruit_NeoPixel::show() : customShow();
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
  _segments[seg].speed = constrain(s, SPEED_MIN, SPEED_MAX);
}

void WS2812FX::increaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(_seg->speed + s, SPEED_MIN, SPEED_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::decreaseSpeed(uint8_t s) {
  uint16_t newSpeed = constrain(_seg->speed - s, SPEED_MIN, SPEED_MAX);
  setSpeed(newSpeed);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setColor((((uint32_t)w << 24)| ((uint32_t)r << 16) | ((uint32_t)g << 8)| ((uint32_t)b)));
}

void WS2812FX::setColor(uint32_t c) {
  setColor(0, c);
}

void WS2812FX::setColor(uint8_t seg, uint32_t c) {
  _segments[seg].colors[0] = c;
}

void WS2812FX::setColors(uint8_t seg, uint32_t* c) {
  for(uint8_t i=0; i<MAX_NUM_COLORS; i++) {
    _segments[seg].colors[i] = c[i];
  }
}

void WS2812FX::setBrightness(uint8_t b) {
//b = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  Adafruit_NeoPixel::setBrightness(b);
  show();
}

void WS2812FX::increaseBrightness(uint8_t s) {
//s = constrain(getBrightness() + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(getBrightness() + s);
}

void WS2812FX::decreaseBrightness(uint8_t s) {
//s = constrain(getBrightness() - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(getBrightness() - s);
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
  uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
  setLength(seglen + s);
}

void WS2812FX::decreaseLength(uint16_t s) {
  uint16_t seglen = _segments[0].stop - _segments[0].start + 1;
  fill(BLACK, _segments[0].start, seglen);
  show();

  if (s < seglen) setLength(seglen - s);
}

bool WS2812FX::isRunning() {
  return _running;
}

bool WS2812FX::isTriggered() {
  return _triggered;
}

bool WS2812FX::isFrame(void) {
  return isFrame(0);
}

bool WS2812FX::isFrame(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return false; // segment not active
  return (_segment_runtimes[ptr - _active_segments].aux_param2 & FRAME);
}

bool WS2812FX::isCycle() {
  return isCycle(0);
}

bool WS2812FX::isCycle(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return false; // segment not active
  return (_segment_runtimes[ptr - _active_segments].aux_param2 & CYCLE);
}

void WS2812FX::setCycle() {
  SET_CYCLE;
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

uint16_t WS2812FX::getNumBytes(void) {
  return numBytes;
}

uint8_t WS2812FX::getNumBytesPerPixel(void) {
  return (wOffset == rOffset) ? 3 : 4; // 3=RGB, 4=RGBW
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
  return _seg;
}

WS2812FX::Segment* WS2812FX::getSegment(uint8_t seg) {
  return &_segments[seg];
}

WS2812FX::Segment* WS2812FX::getSegments(void) {
  return _segments;
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntime(void) {
  return _seg_rt;
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntime(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return NULL; // segment not active
  return &_segment_runtimes[ptr - _active_segments];
}

WS2812FX::Segment_runtime* WS2812FX::getSegmentRuntimes(void) {
  return _segment_runtimes;
}

uint8_t* WS2812FX::getActiveSegments(void) {
  return _active_segments;
}

const __FlashStringHelper* WS2812FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _names[m];
  } else {
    return F("");
  }
}

void WS2812FX::setSegment() {
  setSegment(0, 0, getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n) {
  setSegment(n, 0, getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start) {
  setSegment(n, start, getLength()-1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop) {
  setSegment(n, start, stop, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode) {
  setSegment(n, start, stop, mode, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color) {
  setSegment(n, start, stop, mode, color, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed) {
  setSegment(n, start, stop, mode, color, speed, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse) {
  setSegment(n, start, stop, mode, color, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options) {
  uint32_t colors[] = {color, 0, 0};
  setSegment(n, start, stop, mode, colors, speed, options);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[]) {
  setSegment(n, start, stop, mode, colors, DEFAULT_SPEED, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed) {
  setSegment(n, start, stop, mode, colors, speed, NO_OPTIONS);
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse) {
  setSegment(n, start, stop, mode, colors, speed, (uint8_t)(reverse ? REVERSE : NO_OPTIONS));
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options) {
  if(n < _segments_len) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].options = options;

    setColors(n, (uint32_t*)colors);

    if(n < _active_segments_len) addActiveSegment(n);
  }
}

void WS2812FX::setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed) {
  setIdleSegment(n, start, stop, mode, color, speed, NO_OPTIONS);
}

void WS2812FX::setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options) {
  uint32_t colors[] = {color, 0, 0};
  setIdleSegment(n, start, stop, mode, colors, speed, options);
}

void WS2812FX::setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options) {
  setSegment(n, start, stop, mode, colors, speed, options);
  if(n < _active_segments_len) removeActiveSegment(n);;
}

void WS2812FX::addActiveSegment(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr != NULL) return; // segment already active
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == INACTIVE_SEGMENT) {
      _active_segments[i] = seg;
      resetSegmentRuntime(seg);
      break;
    }
  }
}

void WS2812FX::removeActiveSegment(uint8_t seg) {
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == seg) {
      _active_segments[i] = INACTIVE_SEGMENT;
    }
  }
}

void WS2812FX::swapActiveSegment(uint8_t oldSeg, uint8_t newSeg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, newSeg, _active_segments_len);
  if(ptr != NULL) return; // if newSeg is already active, don't swap
  for(uint8_t i=0; i<_active_segments_len; i++) {
    if(_active_segments[i] == oldSeg) {
      _active_segments[i] = newSeg;

      // reset all runtime parameters EXCEPT next_time,
      // allowing the current animation frame to complete
      __attribute__ ((unused)) segment_runtime seg_rt = _segment_runtimes[i];
      seg_rt.counter_mode_step = 0;
      seg_rt.counter_mode_call = 0;
      seg_rt.aux_param = 0;
      seg_rt.aux_param2 = 0;
      seg_rt.aux_param3 = 0;
      break;
    }
  }
}

bool WS2812FX::isActiveSegment(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr != NULL) return true;
  return false;
}

void WS2812FX::resetSegments() {
  resetSegmentRuntimes();
  memset(_segments, 0, _segments_len * sizeof(Segment));
  memset(_active_segments, INACTIVE_SEGMENT, _active_segments_len);
  _num_segments = 0;
}

void WS2812FX::resetSegmentRuntimes() {
  memset(_segment_runtimes, 0, _active_segments_len * sizeof(Segment_runtime));
}

void WS2812FX::resetSegmentRuntime(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return; // segment not active
  memset(&_segment_runtimes[ptr - _active_segments], 0, sizeof(Segment_runtime));
}

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

void WS2812FX::setRandomSeed(uint16_t seed) {
  _rand16seed = seed;
}

// fast 8-bit random number generator shamelessly borrowed from FastLED
uint8_t WS2812FX::random8() {
  _rand16seed = (_rand16seed * 2053) + 13849;
  return (uint8_t)((_rand16seed + (_rand16seed >> 8)) & 0xFF);
}

// note random8(lim) generates numbers in the range 0 to (lim -1)
uint8_t WS2812FX::random8(uint8_t lim) {
  uint8_t r = random8();
  r = ((uint16_t)r * lim) >> 8;
  return r;
}

uint16_t WS2812FX::random16() {
  return (uint16_t)random8() * 256 + random8();
}

// note random16(lim) generates numbers in the range 0 to (lim - 1)
uint16_t WS2812FX::random16(uint16_t lim) {
  uint16_t r = random16();
  r = ((uint32_t)r * lim) >> 16;
  return r;
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
  uint8_t bytesPerPixel = getNumBytesPerPixel(); // 3=RGB, 4=RGBW
  for(uint16_t i=0; i <numBytes; i += bytesPerPixel) {
    intensities[0] += pixels[i];
    intensities[1] += pixels[i + 1];
    intensities[2] += pixels[i + 2];
    if(bytesPerPixel == 4) intensities[3] += pixels[i + 3]; // for RGBW LEDs
  }
  return intensities;
}


/* #####################################################
#
#  Mode Functions
#
##################################################### */

/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void) {
  fill(_seg->colors[0], _seg->start, _seg_len);
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Blink/strobe function
 * Alternate between color1 and color2
 * if(strobe == true) then create a strobe effect
 */
uint16_t WS2812FX::blink(uint32_t color1, uint32_t color2, bool strobe) {
  if(_seg_rt->counter_mode_call & 1) {
    uint32_t color = (IS_REVERSE) ? color1 : color2; // off
    fill(color, _seg->start, _seg_len);
    SET_CYCLE;
    return strobe ? _seg->speed - 20 : (_seg->speed / 2);
  } else {
    uint32_t color = (IS_REVERSE) ? color2 : color1; // on
    fill(color, _seg->start, _seg_len);
    return strobe ? 20 : (_seg->speed / 2);
  }
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  return blink(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  return blink(color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], false);
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
  return blink(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
  return blink(color_wheel(_seg_rt->counter_mode_call & 0xFF), _seg->colors[1], true);
}


/*
 * Color wipe function
 * LEDs are turned on (color1) in sequence, then turned off (color2) in sequence.
 * if (bool rev == true) then LEDs are turned off in reverse order
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2, bool rev) {
  if(_seg_rt->counter_mode_step < _seg_len) {
    uint32_t led_offset = _seg_rt->counter_mode_step;
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - led_offset, color1);
    } else {
      setPixelColor(_seg->start + led_offset, color1);
    }
  } else {
    uint32_t led_offset = _seg_rt->counter_mode_step - _seg_len;
    if((IS_REVERSE && !rev) || (!IS_REVERSE && rev)) {
      setPixelColor(_seg->stop - led_offset, color2);
    } else {
      setPixelColor(_seg->start + led_offset, color2);
    }
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (_seg_len * 2);

  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / (_seg_len * 2));
}

/*
 * Lights all LEDs one after another.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(_seg->colors[0], _seg->colors[1], false);
}

uint16_t WS2812FX::mode_color_wipe_inv(void) {
  return color_wipe(_seg->colors[1], _seg->colors[0], false);
}

uint16_t WS2812FX::mode_color_wipe_rev(void) {
  return color_wipe(_seg->colors[0], _seg->colors[1], true);
}

uint16_t WS2812FX::mode_color_wipe_rev_inv(void) {
  return color_wipe(_seg->colors[1], _seg->colors[0], true);
}


/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(_seg_rt->counter_mode_step % _seg_len == 0) { // aux_param will store our random color wheel index
    _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param);
  }
  uint32_t color = color_wheel(_seg_rt->aux_param);
  return color_wipe(color, color, false) * 2;
}


/*
 * Random color introduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(_seg_rt->counter_mode_step % _seg_len == 0) { // aux_param will store our random color wheel index
    _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param);
  }
  uint32_t color = color_wheel(_seg_rt->aux_param);
  return color_wipe(color, color, true) * 2;
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param); // aux_param will store our random color wheel index
  uint32_t color = color_wheel(_seg_rt->aux_param);
  fill(color, _seg->start, _seg_len);
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 */
uint16_t WS2812FX::mode_single_dynamic(void) {
  if(_seg_rt->counter_mode_call == 0) {
    for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
      setPixelColor(i, color_wheel(random8()));
    }
  }

  setPixelColor(_seg->start + random16(_seg_len), color_wheel(random8()));
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void) {
  for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
    setPixelColor(i, color_wheel(random8()));
  }
  SET_CYCLE;
  return _seg->speed;
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  int lum = _seg_rt->counter_mode_step;
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

  uint32_t color =  color_blend(_seg->colors[1], _seg->colors[0], lum);
  fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step += 2;
  if(_seg_rt->counter_mode_step > (512-15)) {
    _seg_rt->counter_mode_step = 15;
    SET_CYCLE;
  }
  return delay;
}


/*
 * Fades the LEDs between two colors
 */
uint16_t WS2812FX::mode_fade(void) {
  int lum = _seg_rt->counter_mode_step;
  if(lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0

  uint32_t color = color_blend(_seg->colors[1], _seg->colors[0], lum);
  fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step += 4;
  if(_seg_rt->counter_mode_step > 511) {
    _seg_rt->counter_mode_step = 0;
    SET_CYCLE;
  }
  return (_seg->speed / 128);
}


/*
 * scan function - runs a block of pixels back and forth.
 */
uint16_t WS2812FX::scan(uint32_t color1, uint32_t color2, bool dual) {
  int8_t dir = _seg_rt->aux_param ? -1 : 1;
  uint8_t size = 1 << SIZE_OPTION;

  fill(color2, _seg->start, _seg_len);

  for(uint8_t i = 0; i < size; i++) {
    if(IS_REVERSE || dual) {
      setPixelColor(_seg->stop - _seg_rt->counter_mode_step - i, color1);
    }
    if(!IS_REVERSE || dual) {
      setPixelColor(_seg->start + _seg_rt->counter_mode_step + i, color1);
    }
  }

  _seg_rt->counter_mode_step += dir;
  if(_seg_rt->counter_mode_step == 0) {
    _seg_rt->aux_param = 0;
    SET_CYCLE;
  }
  if(_seg_rt->counter_mode_step >= (uint16_t)(_seg_len - size)) _seg_rt->aux_param = 1;

  return (_seg->speed / (_seg_len * 2));
}


/*
 * Runs a block of pixels back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  return scan(_seg->colors[0], _seg->colors[1], false);
}


/*
 * Runs two blocks of pixels back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  return scan(_seg->colors[0], _seg->colors[1], true);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(_seg_rt->counter_mode_step);
  fill(color, _seg->start, _seg_len);

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

  if(_seg_rt->counter_mode_step == 0)  SET_CYCLE;

  return (_seg->speed / 256);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < _seg_len; i++) {
	  uint32_t color = color_wheel(((i * 256 / _seg_len) + _seg_rt->counter_mode_step) & 0xFF);
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - i, color);
    } else {
      setPixelColor(_seg->start + i, color);
    }
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;

  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / 256);
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  uint8_t sizeCnt = 1 << SIZE_OPTION;
  uint8_t sizeCnt2 = sizeCnt + sizeCnt;
  uint8_t sizeCnt3 = sizeCnt2 + sizeCnt;
  uint16_t index = _seg_rt->counter_mode_step % sizeCnt3;
  for(uint16_t i=0; i < _seg_len; i++, index++) {
    index = index % sizeCnt3;

    uint32_t color = color3;
    if(index < sizeCnt) color = color1;
    else if(index < sizeCnt2) color = color2;

    if(IS_REVERSE) {
      setPixelColor(_seg->start + i, color);
    } else {
      setPixelColor(_seg->stop - i, color);
    }
  }

  _seg_rt->counter_mode_step++;
  if(_seg_rt->counter_mode_step % _seg_len == 0) SET_CYCLE;

  return (_seg->speed / _seg_len);
}


/*
 * Tricolor chase mode
 */
uint16_t WS2812FX::mode_tricolor_chase(void) {
  return tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX::mode_circus_combustus(void) {
  return tricolor_chase(RED, WHITE, BLACK);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  return tricolor_chase(_seg->colors[0], _seg->colors[1], _seg->colors[1]);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  _seg_rt->aux_param = (_seg_rt->aux_param + 1) & 0xFF;
  uint32_t color = color_wheel(_seg_rt->aux_param);
  return tricolor_chase(color, _seg->colors[1], _seg->colors[1]);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
  uint8_t size = 1 << SIZE_OPTION;
  uint8_t sineIncr = max(1, (256 / _seg_len) * size);
  for(uint16_t i=0; i < _seg_len; i++) {
    int lum = (int)sine8(((i + _seg_rt->counter_mode_step) * sineIncr));
    uint32_t color = color_blend(_seg->colors[0], _seg->colors[1], lum);
    if(IS_REVERSE) {
      setPixelColor(_seg->start + i, color);
    } else {
      setPixelColor(_seg->stop - i,  color);
    }
  }
  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % 256;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
  return (_seg->speed / _seg_len);
}


/*
 * twinkle function
 */
uint16_t WS2812FX::twinkle(uint32_t color1, uint32_t color2) {
  if(_seg_rt->counter_mode_step == 0) {
    fill(color2, _seg->start, _seg_len);
    uint16_t min_leds = (_seg_len / 4) + 1; // make sure, at least one LED is on
    _seg_rt->counter_mode_step = random(min_leds, min_leds * 2);
    SET_CYCLE;
  }

  setPixelColor(_seg->start + random16(_seg_len), color1);

  _seg_rt->counter_mode_step--;
  return (_seg->speed / _seg_len);
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void) {
  return twinkle(_seg->colors[0], _seg->colors[1]);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle_random(void) {
  return twinkle(color_wheel(random8()), _seg->colors[1]);
}


/*
 * fade out functions
 */
void WS2812FX::fade_out() {
  return fade_out(_seg->colors[1]);
}

void WS2812FX::fade_out(uint32_t targetColor) {
  static const uint8_t rateMapH[] = {0, 1, 1, 1, 2, 3, 4, 6};
  static const uint8_t rateMapL[] = {0, 2, 3, 8, 8, 8, 8, 8};

  uint8_t rate  = FADE_RATE;
  uint8_t rateH = rateMapH[rate];
  uint8_t rateL = rateMapL[rate];

  uint32_t color = targetColor;
  int w2 = (color >> 24) & 0xff;
  int r2 = (color >> 16) & 0xff;
  int g2 = (color >>  8) & 0xff;
  int b2 =  color        & 0xff;

  for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
    color = getPixelColor(i); // current color
    if(rate == 0) { // old fade-to-black algorithm
      setPixelColor(i, (color >> 1) & 0x7F7F7F7F);
    } else { // new fade-to-color algorithm
      int w1 = (color >> 24) & 0xff;
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
uint32_t WS2812FX::color_blend(uint32_t color1, uint32_t color2, uint8_t blendAmt) {
  uint32_t blendedColor;
  blend((uint8_t*)&blendedColor, (uint8_t*)&color1, (uint8_t*)&color2, sizeof(uint32_t), blendAmt);
  return blendedColor;
}

uint8_t* WS2812FX::blend(uint8_t *dest, uint8_t *src1, uint8_t *src2, uint16_t cnt, uint8_t blendAmt) {
  if(blendAmt == 0) {
    memmove(dest, src1, cnt);
  } else if(blendAmt == 255) {
    memmove(dest, src2, cnt);
  } else {
    for(uint16_t i=0; i<cnt; i++) {
//    dest[i] = map(blendAmt, 0, 255, src1[i], src2[i]);
      dest[i] =  blendAmt * ((int)src2[i] - (int)src1[i]) / 256 + src1[i]; // map() function
    }
  }
  return dest;
}

/*
 * twinkle_fade function
 */
uint16_t WS2812FX::twinkle_fade(uint32_t color) {
  fade_out();

  if(random8(3) == 0) {
    uint8_t size = 1 << SIZE_OPTION;
    uint16_t index = _seg->start + random16(_seg_len - size + 1);
    fill(color, index, size);
    SET_CYCLE;
  }
  return (_seg->speed / 8);
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  return twinkle_fade(_seg->colors[0]);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade_random(void) {
  return twinkle_fade(color_wheel(random8()));
}

/*
 * Sparkle function
 * color1 = background color
 * color2 = sparkle color
 */
uint16_t WS2812FX::sparkle(uint32_t color1, uint32_t color2) {
  if(_seg_rt->counter_mode_step == 0) {
    fill(color1, _seg->start, _seg_len);
  }

  uint8_t size = 1 << SIZE_OPTION;
  fill(color1, _seg->start + _seg_rt->aux_param3, size);

  _seg_rt->aux_param3 = random16(_seg_len - size + 1); // aux_param3 stores the random led index
  fill(color2, _seg->start + _seg_rt->aux_param3, size);

  SET_CYCLE;
  return (_seg->speed / 32);
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void) {
  return sparkle(_seg->colors[1], _seg->colors[0]);
}


/*
 * Lights all LEDs in the color. Flashes white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  return sparkle(_seg->colors[0], WHITE);
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/arduino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  fill(_seg->colors[0], _seg->start, _seg_len);

  uint8_t size = 1 << SIZE_OPTION;
  for(uint8_t i=0; i<8; i++) {
    fill(WHITE, _seg->start + random16(_seg_len - size + 1), size);
  }

  SET_CYCLE;
  return (_seg->speed / 32);
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  fill(_seg->colors[1], _seg->start, _seg_len);

  uint16_t delay = 200 + ((9 - (_seg->speed % 10)) * 100);
  uint16_t count = 2 * ((_seg->speed / 100) + 1);
  if(_seg_rt->counter_mode_step < count) {
    if((_seg_rt->counter_mode_step & 1) == 0) {
      fill(_seg->colors[0], _seg->start, _seg_len);
      delay = 20;
    } else {
      delay = 50;
    }
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % (count + 1);
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
  return delay;
}


/*
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  uint8_t size = 1 << SIZE_OPTION;
  for(uint8_t i=0; i<size; i++) {
    uint16_t a = (_seg_rt->counter_mode_step + i) % _seg_len;
    uint16_t b = (a + size) % _seg_len;
    uint16_t c = (b + size) % _seg_len;
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - a, color1);
      setPixelColor(_seg->stop - b, color2);
      setPixelColor(_seg->stop - c, color3);
    } else {
      setPixelColor(_seg->start + a, color1);
      setPixelColor(_seg->start + b, color2);
      setPixelColor(_seg->start + c, color3);
    }
  }

  if(_seg_rt->counter_mode_step + (size * 3) == _seg_len) SET_CYCLE;

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
  return (_seg->speed / _seg_len);
}


/*
 * Bicolor chase mode
 */
uint16_t WS2812FX::mode_bicolor_chase(void) {
  return chase(_seg->colors[0], _seg->colors[1], _seg->colors[2]);
}


/*
 * White running on _color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  return chase(_seg->colors[0], WHITE, WHITE);
}


/*
 * Black running on _color.
 */
uint16_t WS2812FX::mode_chase_blackout(void) {
  return chase(_seg->colors[0], BLACK, BLACK);
}


/*
 * _color running on white.
 */
uint16_t WS2812FX::mode_chase_white(void) {
  return chase(WHITE, _seg->colors[0], _seg->colors[0]);
}


/*
 * White running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(_seg_rt->counter_mode_step == 0) {
    _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param);
  }
  return chase(color_wheel(_seg_rt->aux_param), WHITE, WHITE);
}


/*
 * Rainbow running on white.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint16_t n = _seg_rt->counter_mode_step;
  uint16_t m = (_seg_rt->counter_mode_step + 1) % _seg_len;
  uint32_t color2 = color_wheel(((n * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / _seg_len) + (_seg_rt->counter_mode_call & 0xFF)) & 0xFF);

  return chase(WHITE, color2, color3);
}


/*
 * White running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / _seg_len;
  uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, WHITE, WHITE);
}


/*
 * Black running on rainbow.
 */
uint16_t WS2812FX::mode_chase_blackout_rainbow(void) {
  uint8_t color_sep = 256 / _seg_len;
  uint8_t color_index = _seg_rt->counter_mode_call & 0xFF;
  uint32_t color = color_wheel(((_seg_rt->counter_mode_step * color_sep) + color_index) & 0xFF);

  return chase(color, BLACK, BLACK);
}

/*
 * running white flashes function.
 * color1 = background color
 * color2 = flash color
 */
uint16_t WS2812FX::chase_flash(uint32_t color1, uint32_t color2) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = _seg_rt->counter_mode_call % ((flash_count * 2) + 1);

  if(flash_step < (flash_count * 2)) {
    uint32_t color = (flash_step % 2 == 0) ? color2 : color1;
    uint16_t n = _seg_rt->counter_mode_step;
    uint16_t m = (_seg_rt->counter_mode_step + 1) % _seg_len;
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - n, color);
      setPixelColor(_seg->stop - m, color);
    } else {
      setPixelColor(_seg->start + n, color);
      setPixelColor(_seg->start + m, color);
    }
    return 30;
  } else {
    _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
    if(_seg_rt->counter_mode_step == 0) {
      // update aux_param so mode_chase_flash_random() will select the next color
      _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param);
      SET_CYCLE;
    }
  }
  return (_seg->speed / _seg_len);
}

/*
 * White flashes running on _color.
 */
uint16_t WS2812FX::mode_chase_flash(void) {
  return chase_flash(_seg->colors[0], WHITE);
}


/*
 * White flashes running, followed by random color.
 */
uint16_t WS2812FX::mode_chase_flash_random(void) {
  return chase_flash(color_wheel(_seg_rt->aux_param), WHITE);
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  uint8_t size = 2 << SIZE_OPTION;
  uint32_t color = (_seg_rt->counter_mode_step & size) ? color1 : color2;

  if(IS_REVERSE) {
    copyPixels(_seg->start, _seg->start + 1, _seg_len - 1);
    setPixelColor(_seg->stop, color);
  } else {
    copyPixels(_seg->start + 1, _seg->start, _seg_len - 1);
    setPixelColor(_seg->start, color);
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;
  return (_seg->speed / _seg_len);
}


/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(_seg->colors[0], _seg->colors[1]);
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
  uint8_t size = 2 << SIZE_OPTION;
  if((_seg_rt->counter_mode_step) % size == 0) {
    _seg_rt->aux_param = get_random_wheel_index(_seg_rt->aux_param);
  }

  uint32_t color = color_wheel(_seg_rt->aux_param);

  return running(color, color);
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {
  fade_out();

  if(_seg_rt->counter_mode_step < _seg_len) {
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
    } else {
      setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
    }
  } else {
    uint16_t index = (_seg_len * 2) - _seg_rt->counter_mode_step - 2;
    if(IS_REVERSE) {
      setPixelColor(_seg->stop - index, _seg->colors[0]);
    } else {
      setPixelColor(_seg->start + index, _seg->colors[0]);
    }
  }

  _seg_rt->counter_mode_step++;
  if(_seg_rt->counter_mode_step >= (uint16_t)((_seg_len * 2) - 2)) {
    _seg_rt->counter_mode_step = 0;
    SET_CYCLE;
  }

  return (_seg->speed / (_seg_len * 2));
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {
  fade_out();

  if(IS_REVERSE) {
    setPixelColor(_seg->stop - _seg_rt->counter_mode_step, _seg->colors[0]);
  } else {
    setPixelColor(_seg->start + _seg_rt->counter_mode_step, _seg->colors[0]);
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) % _seg_len;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / _seg_len);
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::fireworks(uint32_t color) {
  fade_out();

// for better performance, manipulate the Adafruit_NeoPixels pixels[] array directly
  uint8_t *pixels = getPixels();
  uint8_t bytesPerPixel = getNumBytesPerPixel(); // 3=RGB, 4=RGBW
  uint16_t startPixel = _seg->start * bytesPerPixel + bytesPerPixel;
  uint16_t stopPixel = _seg->stop * bytesPerPixel;
  for(uint16_t i=startPixel; i <stopPixel; i++) {
    uint16_t tmpPixel = (pixels[i - bytesPerPixel] >> 2) +
      pixels[i] +
      (pixels[i + bytesPerPixel] >> 2);
    pixels[i] =  tmpPixel > 255 ? 255 : tmpPixel;
  }

  uint8_t size = 2 << SIZE_OPTION;
  if(!_triggered) {
    for(uint16_t i=0; i<max(1, _seg_len/20); i++) {
      if(random8(10) == 0) {
        uint16_t index = _seg->start + random16(_seg_len - size + 1);
        fill(color, index, size);
        SET_CYCLE;
      }
    }
  } else {
    for(uint16_t i=0; i<max(1, _seg_len/10); i++) {
      uint16_t index = _seg->start + random16(_seg_len - size + 1);
      fill(color, index, size);
      SET_CYCLE;
    }
  }

  return (_seg->speed / _seg_len);
}

/*
 * Firework sparks.
 */
uint16_t WS2812FX::mode_fireworks(void) {
  uint32_t color = BLACK;
  do { // randomly choose a non-BLACK color from the colors array
    color = _seg->colors[random8(MAX_NUM_COLORS)];
  } while (color == BLACK);
  return fireworks(color);
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
  byte w = (_seg->colors[0] >> 24) & 0xFF;
  byte r = (_seg->colors[0] >> 16) & 0xFF;
  byte g = (_seg->colors[0] >>  8) & 0xFF;
  byte b = (_seg->colors[0]        & 0xFF);
  byte lum = max(w, max(r, max(g, b))) / rev_intensity;
  for(uint16_t i=_seg->start; i <= _seg->stop; i++) {
    int flicker = random8(lum);
    setPixelColor(i, max(r - flicker, 0), max(g - flicker, 0), max(b - flicker, 0), max(w - flicker, 0));
  }

  SET_CYCLE;
  return (_seg->speed / _seg_len);
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
  return fire_flicker(1);
}

// An adaptation of Mark Kriegsman's FastLED twinkeFOX effect
// https://gist.github.com/kriegsman/756ea6dcae8e30845b5a
uint16_t WS2812FX::mode_twinkleFOX(void) {
  uint16_t mySeed = 0; // reset the random number generator seed

  // Get and translate the segment's size option
  uint8_t size = 1 << ((_seg->options >> 1) & 0x03); // 1,2,4,8

  // Get the segment's colors array values
  uint32_t color0 = _seg->colors[0];
  uint32_t color1 = _seg->colors[1];
  uint32_t color2 = _seg->colors[2];
  uint32_t blendedColor;

  for (uint16_t i = _seg->start; i <= _seg->stop; i+=size) {
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
    uint8_t blendIndex = (initValue + (_seg_rt->counter_mode_call * incrValue)) & 0xff; // 0-255
    // Index into the built-in Adafruit_NeoPixel sine table to lookup the blend amount
    uint8_t blendAmt = Adafruit_NeoPixel::sine8(blendIndex); // 0-255

    // If colors[0] is BLACK, blend random colors
    if(color0 == BLACK) {
      blendedColor = color_blend(color_wheel(initValue), color1, blendAmt);
    // If colors[2] isn't BLACK, choose to blend colors[0]/colors[1] or colors[1]/colors[2]
    // (which color pair to blend is picked randomly)
    } else if((color2 != BLACK) && (initValue < 128) == 0) {
      blendedColor = color_blend(color2, color1, blendAmt);
    // Otherwise always blend colors[0]/colors[1]
    } else {
      blendedColor = color_blend(color0, color1, blendAmt);
    }

    // Assign the new color to the number of LEDs specified by the SIZE option
    for(uint8_t j=0; j<size; j++) {
      if((i + j) <= _seg->stop) {
        setPixelColor(i + j, blendedColor);
      }
    }
  }
  setCycle();
  return _seg->speed / 32;
}

// A combination of the Fireworks effect and the running effect
// to create an effect that looks like rain.
uint16_t WS2812FX::mode_rain(void) {
  // randomly choose colors[0] or colors[2]
  uint32_t rainColor = (random8() & 1) == 0 ? _seg->colors[0] : _seg->colors[2];
  // if colors[0] == colors[1], choose a random color
  if(_seg->colors[0] == _seg->colors[1]) rainColor = color_wheel(random8());

  // run the fireworks effect to create a "raindrop"
  fireworks(rainColor);

  // shift everything two pixels
  if(IS_REVERSE) {
    copyPixels(_seg->start, _seg->start + 2, _seg_len - 2);
  } else {
    copyPixels(_seg->start + 2, _seg->start, _seg_len - 2);
  }

  return (_seg->speed / 16);
}

/*
 * Custom modes
 */
uint16_t WS2812FX::mode_custom_0() {
  return customModes[0]();
}
uint16_t WS2812FX::mode_custom_1() {
  return customModes[1]();
}
uint16_t WS2812FX::mode_custom_2() {
  return customModes[2]();
}
uint16_t WS2812FX::mode_custom_3() {
  return customModes[3]();
}
uint16_t WS2812FX::mode_custom_4() {
  return customModes[4]();
}
uint16_t WS2812FX::mode_custom_5() {
  return customModes[5]();
}
uint16_t WS2812FX::mode_custom_6() {
  return customModes[6]();
}
uint16_t WS2812FX::mode_custom_7() {
  return customModes[7]();
}

/*
 * Custom mode helpers
 */
void WS2812FX::setCustomMode(uint16_t (*p)()) {
  customModes[0] = p;
}

uint8_t WS2812FX::setCustomMode(const __FlashStringHelper* name, uint16_t (*p)()) {
  static uint8_t custom_mode_index = 0;
  return setCustomMode(custom_mode_index++, name, p);
}

uint8_t WS2812FX::setCustomMode(uint8_t index, const __FlashStringHelper* name, uint16_t (*p)()) {
  if((uint8_t)(FX_MODE_CUSTOM_0 + index) < MODE_COUNT) {
    _names[FX_MODE_CUSTOM_0 + index] = name; // store the custom mode name
    customModes[index] = p; // store the custom mode

    return (FX_MODE_CUSTOM_0 + index);
  }
  return 0;
}

/*
 * Custom show helper
 */
void WS2812FX::setCustomShow(void (*p)()) {
  customShow = p;
}
