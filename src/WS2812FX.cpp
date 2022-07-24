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
//   for (int j=0; j < 1000; j++) {
//     uint16_t delay = (MODE_PTR(_seg->mode))();
//   }
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
          uint16_t delay = (MODE_PTR(_seg->mode))();
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

// custom setPixelColor() function that bypasses the Adafruit_Neopixel global brightness rigmarole
void WS2812FX::setRawPixelColor(uint16_t n, uint32_t c) {
  if (n < numLEDs) {
    uint8_t *p = (wOffset == rOffset) ? &pixels[n * 3] : &pixels[n * 4]; 
    uint8_t w = (uint8_t)(c >> 24), r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;

    p[wOffset] = w;
    p[rOffset] = r;
    p[gOffset] = g;
    p[bOffset] = b;
  }
}

// custom getPixelColor() function that bypasses the Adafruit_Neopixel global brightness rigmarole
uint32_t WS2812FX::getRawPixelColor(uint16_t n) {
  if (n >= numLEDs) return 0; // Out of bounds, return no color.

  if(wOffset == rOffset) { // RGB
    uint8_t *p = &pixels[n * 3]; 
    return ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
  } else { // RGBW
    uint8_t *p = &pixels[n * 4];
    return ((uint32_t)p[wOffset] << 24) | ((uint32_t)p[rOffset] << 16) | ((uint32_t)p[gOffset] << 8) | (uint32_t)p[bOffset];
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
    return MODE_NAME(m);
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
  for(uint8_t i=0; i<_segments_len; i++) {
    resetSegmentRuntime(i);
  };
}

void WS2812FX::resetSegmentRuntime(uint8_t seg) {
  uint8_t* ptr = (uint8_t*)memchr(_active_segments, seg, _active_segments_len);
  if(ptr == NULL) return; // segment not active
  _segment_runtimes[seg].next_time = 0;
  _segment_runtimes[seg].counter_mode_step = 0;
  _segment_runtimes[seg].counter_mode_call = 0;
  _segment_runtimes[seg].aux_param = 0;
  _segment_runtimes[seg].aux_param2 = 0;
  _segment_runtimes[seg].aux_param3 = 0;
  // don't reset any external data source
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
    MODE_NAME(FX_MODE_CUSTOM_0 + index) = name;
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

/*
 * set a segment runtime's external data source
 */
void WS2812FX::setExtDataSrc(uint8_t seg, uint8_t *src, uint8_t cnt) {
  _segment_runtimes[seg].extDataSrc = src;
  _segment_runtimes[seg].extDataCnt = cnt;
}
