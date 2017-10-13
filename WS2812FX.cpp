/*
  WS2812FX.cpp - Library for WS2812 LED effects.

  Harm Aldick - 2016
  www.aldick.org


  FEATURES
    * A lot of blinken modes and counting
    * WS2812FX can be used as drop-in replacement for Adafruit Neopixel Library

  NOTES
    * Uses the Adafruit Neopixel library. Get it here:
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
*/

#include "Arduino.h"
#include "WS2812FX.h"

//#define CALL_MODE(n) (this->*_mode[n])();

void WS2812FX::init() {
  RESET_RUNTIME;
  Adafruit_NeoPixel::begin();
  setBrightness(_brightness);
  Adafruit_NeoPixel::show();
}

void WS2812FX::service() {
  if(_running || _triggered) {
    unsigned long now = millis(); // Be aware, millis() rolls over every 49 days
    bool doShow = false;
    for(uint8_t i=0; i < _num_segments; i++) {
      _segment_index = i;
      if(now > SEGMENT_RUNTIME.next_time || _triggered) {
        doShow = true;
        uint16_t delay = (this->*_mode[SEGMENT.mode])();
        SEGMENT_RUNTIME.next_time = now + max(delay, 20);
        SEGMENT_RUNTIME.counter_mode_call++;
      }
    }
    if(doShow) Adafruit_NeoPixel::show();
    _triggered = false;
  }
}

void WS2812FX::start() {
  RESET_RUNTIME;
  _running = true;
}

void WS2812FX::stop() {
  _running = false;
  strip_off();
}

void WS2812FX::trigger() {
  _triggered = true;
}

void WS2812FX::setMode(uint8_t m) {
  RESET_RUNTIME;
  _segments[0].mode = constrain(m, 0, MODE_COUNT - 1);
  setBrightness(_brightness);
  //strip_off();
}

void WS2812FX::setSpeed(uint16_t s) {
  RESET_RUNTIME;
  _segments[0].speed = constrain(s, SPEED_MIN, SPEED_MAX);
  //strip_off();
}

void WS2812FX::increaseSpeed(uint8_t s) {
  s = constrain(SEGMENT.speed + s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2812FX::decreaseSpeed(uint8_t s) {
  s = constrain(SEGMENT.speed - s, SPEED_MIN, SPEED_MAX);
  setSpeed(s);
}

void WS2812FX::setColor(uint8_t r, uint8_t g, uint8_t b) {
  setColor(((uint32_t)r << 16) | ((uint32_t)g << 8) | b);
}

void WS2812FX::setColor(uint32_t c) {
  RESET_RUNTIME;
  _segments[0].colors[0] = c;
  setBrightness(_brightness);
  //strip_off();
}

void WS2812FX::setBrightness(uint8_t b) {
  _brightness = constrain(b, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  Adafruit_NeoPixel::setBrightness(_brightness);
  Adafruit_NeoPixel::show();
  delay(1);
}

void WS2812FX::increaseBrightness(uint8_t s) {
  s = constrain(_brightness + s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::decreaseBrightness(uint8_t s) {
  s = constrain(_brightness - s, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
  setBrightness(s);
}

void WS2812FX::setLength(uint16_t b) {
  RESET_RUNTIME;
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
    Adafruit_NeoPixel::setPixelColor(i, 0);
  }
  Adafruit_NeoPixel::show();

  setLength(s);
}

boolean WS2812FX::isRunning() {
  return _running;
}

uint8_t WS2812FX::getMode(void) {
  return _segments[0].mode;
}

uint16_t WS2812FX::getSpeed(void) {
  return _segments[0].speed;
}

uint8_t WS2812FX::getBrightness(void) {
  return _brightness;
}

uint16_t WS2812FX::getLength(void) {
  return _segments[0].stop - _segments[0].start + 1;
}

uint8_t WS2812FX::getModeCount(void) {
  return MODE_COUNT;
}

uint8_t WS2812FX::getNumSegments(void) {
  return _num_segments;
}

uint32_t WS2812FX::getColor(void) {
  return _segments[0].colors[0];
}

WS2812FX::segment* WS2812FX::getSegments(void) {
  return _segments;
}

const __FlashStringHelper* WS2812FX::getModeName(uint8_t m) {
  if(m < MODE_COUNT) {
    return _name[m];
  } else {
    return F("");
  }
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse) {
  if(n < MAX_NUM_SEGMENTS) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].reverse = reverse;
    _segments[n].colors[0] = color;
  }
}

void WS2812FX::setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t colors[], uint16_t speed, bool reverse) {
  if(n < MAX_NUM_SEGMENTS) {
    if(n + 1 > _num_segments) _num_segments = n + 1;
    _segments[n].start = start;
    _segments[n].stop = stop;
    _segments[n].mode = mode;
    _segments[n].speed = speed;
    _segments[n].reverse = reverse;

    for(uint8_t i=0; i<NUM_COLORS; i++) {
      _segments[n].colors[i] = colors[i];
    }
  }
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
  Adafruit_NeoPixel::show();
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
    r = random(256);
    x = abs(pos - r);
    y = 255 - x;
    d = min(x, y);
  }

  return r;
}


/*
 * No blinking. Just plain old static light.
 */
uint16_t WS2812FX::mode_static(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
  }
  return 200;
}


/*
 * Normal blinking. 50% on/off time.
 */
uint16_t WS2812FX::mode_blink(void) {
  uint32_t color = SEGMENT_RUNTIME.counter_mode_call % 2 == 1 ? 0 : SEGMENT.colors[0];
  if(SEGMENT.reverse) color = color == 0 ? SEGMENT.colors[0] : 0;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color);
  }
//return (SEGMENT.speed / 2);
  return 100 + ((1986 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Color wipe function
 */
uint16_t WS2812FX::color_wipe(uint32_t color1, uint32_t color2) {
  uint32_t led_offset = SEGMENT_RUNTIME.counter_mode_step;
  uint32_t color = color1;
  if(SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {
    led_offset = SEGMENT_RUNTIME.counter_mode_step - SEGMENT_LENGTH;
    color = color2;
  }
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - led_offset, color);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + led_offset, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 2);
//return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Lights all LEDs after each other up. Then turns them in
 * that order off. Repeat.
 */
uint16_t WS2812FX::mode_color_wipe(void) {
  return color_wipe(SEGMENT.color[0], 0);
}

uint16_t WS2812FX::mode_color_wipe_inverse(void) {
  return color_wipe(0, SEGMENT.color[0]);
}

/*
 * Turns all LEDs after each other to a random color.
 * Then starts over with another color.
 */
uint16_t WS2812FX::mode_color_wipe_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) { // aux_param will store our random color wheel index
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }

  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, color_wheel(SEGMENT_RUNTIME.aux_param));
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Lights all LEDs in one random color up. Then switches them
 * to the next random color.
 */
uint16_t WS2812FX::mode_random_color(void) {
  SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param); // aux_param will store our random color wheel index

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }
//return (SEGMENT.speed);
  return 100 + ((5000 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Lights every LED in a random color. Changes one random LED after the other
 * to another random color.
 */
uint16_t WS2812FX::mode_single_dynamic(void) {
  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, color_wheel(random(256)));
    }
  }

  Adafruit_NeoPixel::setPixelColor(SEGMENT.start+random(SEGMENT_LENGTH), color_wheel(random(256)));
//return (SEGMENT.speed);
  return 10 + ((5000 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Lights every LED in a random color. Changes all LED at the same time
 * to new random colors.
 */
uint16_t WS2812FX::mode_multi_dynamic(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color_wheel(random(256)));
  }
//return (SEGMENT.speed);
  return 100 + ((5000 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Does the "standby-breathing" of well known i-Devices. Fixed Speed.
 * Use mode "fade" if you like to have something similar with a different speed.
 */
uint16_t WS2812FX::mode_breath(void) {
  //                                      0    1    2   3   4   5   6    7   8   9  10  11   12   13   14   15   16    // step
  uint16_t breath_delay_steps[] =     {   7,   9,  13, 15, 16, 17, 18, 930, 19, 18, 15, 13,   9,   7,   4,   5,  10 }; // magic numbers for breathing LED
  uint8_t breath_brightness_steps[] = { 150, 125, 100, 75, 50, 25, 16,  15, 16, 25, 50, 75, 100, 125, 150, 220, 255 }; // even more magic numbers!

  if(SEGMENT_RUNTIME.counter_mode_call == 0) {
    SEGMENT_RUNTIME.aux_param = breath_brightness_steps[0] + 1; // we use aux_param to store the brightness
  }

  uint8_t breath_brightness = SEGMENT_RUNTIME.aux_param;

  if(SEGMENT_RUNTIME.counter_mode_step < 8) {
    breath_brightness--;
  } else {
    breath_brightness++;
  }

  // update index of current delay when target brightness is reached, start over after the last step
  if(breath_brightness == breath_brightness_steps[SEGMENT_RUNTIME.counter_mode_step]) {
    SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (sizeof(breath_brightness_steps)/sizeof(uint8_t));
  }

  int b = map(breath_brightness, 0, 255, 0, _brightness);  // keep brightness below brightness set by user
  uint8_t red = (SEGMENT.colors[0] >> 16 & 0xFF) * b / _brightness; // modify RGB colors with brightness info
  uint8_t green = (SEGMENT.colors[0] >> 8 & 0xFF) * b / _brightness;
  uint8_t blue = (SEGMENT.colors[0] & 0xFF) * b / _brightness;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, red, green, blue);    // set all LEDs to selected color
  }

  SEGMENT_RUNTIME.aux_param = breath_brightness;
//return (SEGMENT.speed / 550); // another magic number :)
  return breath_delay_steps[SEGMENT_RUNTIME.counter_mode_step];
}


/*
 * Fades the LEDs on and (almost) off again.
 */
uint16_t WS2812FX::mode_fade(void) {
  int b = SEGMENT_RUNTIME.counter_mode_step - 31;
  b = 63 - (abs(b) * 2);
  b = map(b, 0, 64, min(25, _brightness), _brightness);

  uint8_t red = (SEGMENT.colors[0] >> 16 & 0xFF) * b / _brightness; // modify RGB colors with brightness info
  uint8_t green = (SEGMENT.colors[0] >> 8 & 0xFF) * b / _brightness;
  uint8_t blue = (SEGMENT.colors[0] & 0xFF) * b / _brightness;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, red, green, blue);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 64;
//return (SEGMENT.speed / 64);
  return 5 + ((15 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Runs a single pixel back and forth.
 */
uint16_t WS2812FX::mode_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH * 2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, 0);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset); 

  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - led_offset, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step++;
//return (SEGMENT.speed / ((SEGMENT_LENGTH - 1) * 2));
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Runs two pixel back and forth in opposite directions.
 */
uint16_t WS2812FX::mode_dual_scan(void) {
  if(SEGMENT_RUNTIME.counter_mode_step > (SEGMENT_LENGTH*2) - 3) {
    SEGMENT_RUNTIME.counter_mode_step = 0;
  }

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, 0);
  }

  int led_offset = SEGMENT_RUNTIME.counter_mode_step - (SEGMENT_LENGTH - 1);
  led_offset = abs(led_offset);

  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + led_offset, SEGMENT.colors[0]);
  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - led_offset - 1, SEGMENT.colors[0]);

  SEGMENT_RUNTIME.counter_mode_step++;
//return (SEGMENT.speed / ((SEGMENT_LENGTH - 1) * 2));
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Cycles all LEDs at once through a rainbow.
 */
uint16_t WS2812FX::mode_rainbow(void) {
  uint32_t color = color_wheel(SEGMENT_RUNTIME.counter_mode_step);
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 256;
//return (SEGMENT.speed / 256);
  return 1 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Cycles a rainbow over the entire string of LEDs.
 */
uint16_t WS2812FX::mode_rainbow_cycle(void) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel(((i * 256 / SEGMENT_LENGTH) + SEGMENT_RUNTIME.counter_mode_step) % 256));
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 256;
//return (SEGMENT.speed / 256);
  return 1 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Theatre-style crawling lights.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase(void) {
  SEGMENT_RUNTIME.counter_mode_call = SEGMENT_RUNTIME.counter_mode_call % 3;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i % 3) == SEGMENT_RUNTIME.counter_mode_call) {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, SEGMENT.colors[0]);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, SEGMENT.colors[0]);
      }
    } else {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, 0);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, 0);
      }
    }
  }

//return (SEGMENT.speed / 3);
  return 50 + ((500 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Theatre-style crawling lights with rainbow effect.
 * Inspired by the Adafruit examples.
 */
uint16_t WS2812FX::mode_theater_chase_rainbow(void) {
  SEGMENT_RUNTIME.counter_mode_call = SEGMENT_RUNTIME.counter_mode_call % 3;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i % 3) == SEGMENT_RUNTIME.counter_mode_call) {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color_wheel((i + SEGMENT_RUNTIME.counter_mode_step) % 256));
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel((i + SEGMENT_RUNTIME.counter_mode_step) % 256));
      }
    } else {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, 0);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, 0);
      }
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 256;
//return (SEGMENT.speed / 3);
  return 50 + ((500 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Running lights effect with smooth sine transition.
 */
uint16_t WS2812FX::mode_running_lights(void) {
  uint8_t r = ((SEGMENT.colors[0] >> 16) & 0xFF);
  uint8_t g = ((SEGMENT.colors[0] >> 8) & 0xFF);
  uint8_t b = (SEGMENT.colors[0] & 0xFF);

  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    int s = (sin(i + SEGMENT_RUNTIME.counter_mode_call) * 127) + 128;
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, (((uint32_t)(r * s)) / 255), (((uint32_t)(g * s)) / 255), (((uint32_t)(b * s)) / 255));
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, (((uint32_t)(r * s)) / 255), (((uint32_t)(g * s)) / 255), (((uint32_t)(b * s)) / 255));
    }
  }
//return (SEGMENT.speed / (SEGMENT_LENGTH / 2));
  return 35 + ((350 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * twinkle function
 */
uint16_t WS2812FX::twinkle(uint32_t color) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, 0);
    }
    uint16_t min_leds = max(1, SEGMENT_LENGTH / 5); // make sure, at least one LED is on
    uint16_t max_leds = max(1, SEGMENT_LENGTH / 2); // make sure, at least one LED is on
    SEGMENT_RUNTIME.counter_mode_step = random(min_leds, max_leds);
  }

  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);

  SEGMENT_RUNTIME.counter_mode_step--;
//return (SEGMENT.speed);
  return 50 + ((1986 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}

/*
 * Blink several LEDs on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle(void) {
  uint32_t color = SEGMENT.colors[0];
  return twinkle(color);
}

/*
 * Blink several LEDs in random colors on, reset, repeat.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_twinkle_random(void) {
  uint32_t color = color_wheel(random(256));
  return twinkle(color);
}


/*
 * twinkle_fade function
 */
uint16_t WS2812FX::twinkle_fade(uint32_t color) {

  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    uint32_t px_rgb = Adafruit_NeoPixel::getPixelColor(SEGMENT.start+i);

    byte px_r = (px_rgb & 0x00FF0000) >> 16;
    byte px_g = (px_rgb & 0x0000FF00) >>  8;
    byte px_b = (px_rgb & 0x000000FF) >>  0;

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, px_r, px_g, px_b);
  }

  if(random(3) == 0) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
  }
//return (SEGMENT.speed / 8);
  return 100 + ((100 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Blink several LEDs on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade(void) {
  uint32_t color = SEGMENT.colors[0];
  return twinkle_fade(color);
}


/*
 * Blink several LEDs in random colors on, fading out.
 */
uint16_t WS2812FX::mode_twinkle_fade_random(void) {
  uint32_t color = color_wheel(random(256));
  return twinkle_fade(color);
}


/*
 * Blinks one LED at a time.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, 0);
  }
  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), SEGMENT.colors[0]);
//return (SEGMENT.speed);
  return 10 + ((200 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * Lights all LEDs in the color. Flashes single white pixels randomly.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_flash_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
  }

//uint16_t delay = SEGMENT.speed;
  uint16_t delay = 20 + ((200 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
  if(random(10) < 2) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), 255, 255, 255);
    delay = 20;
  } 
  return delay;
}


/*
 * Like flash sparkle. With more flash.
 * Inspired by www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
 */
uint16_t WS2812FX::mode_hyper_sparkle(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
  }

//uint16_t delay = SEGMENT.speed;
  uint16_t delay = 15 + ((120 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
  if(random(10) < 4) {
    for(uint16_t i=0; i < max(1, SEGMENT_LENGTH/3); i++) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), 255, 255, 255);
    }
    delay = 20;
  }
  return delay;
}


/*
 * Classic Strobe effect.
 */
uint16_t WS2812FX::mode_strobe(void) {
//uint16_t delay = SEGMENT.speed - 20;
  uint16_t delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
  if(SEGMENT_RUNTIME.counter_mode_call % 2 == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
    }
    delay = 20;
  } else {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, 0);
    }
  }
  return delay;
}


/*
 * Strobe effect with different strobe count and pause, controlled by speed.
 */
uint16_t WS2812FX::mode_multi_strobe(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, 0);
  }

//uint16_t delay = SEGMENT.speed / (2 * ((SEGMENT.speed / 10) + 1));
  uint16_t delay = 100 + ((9 - (SEGMENT.speed % 10)) * 125);
  if(SEGMENT_RUNTIME.counter_mode_step < (2 * ((SEGMENT.speed / 10) + 1))) {
    if(SEGMENT_RUNTIME.counter_mode_step % 2 == 0) {
      for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
        Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
      }
      delay = 20;
    } else {
      delay = 50;
    }
  }
  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % ((2 * ((SEGMENT.speed / 10) + 1)) + 1);
  return delay;
}


/*
 * Classic Strobe effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_strobe_rainbow(void) {
//uint16_t delay = SEGMENT.speed - 20;
  uint16_t delay = 50 + ((1986 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
  if(SEGMENT_RUNTIME.counter_mode_call % 2 == 0) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, color_wheel(SEGMENT_RUNTIME.counter_mode_call % 256));
    }
    delay = 20;
  } else {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, 0);
    }
  }
  return delay;
}


/*
 * Classic Blink effect. Cycling through the rainbow.
 */
uint16_t WS2812FX::mode_blink_rainbow(void) {
  if(SEGMENT_RUNTIME.counter_mode_call % 2 == 1) {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, color_wheel(SEGMENT_RUNTIME.counter_mode_call % 256));
    }
  } else {
    for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
      Adafruit_NeoPixel::setPixelColor(i, 0);
    }
  }
//return (SEGMENT.speed / 2);
  return 100 + ((1986 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}


/*
 * _color running on white.
 */
uint16_t WS2812FX::mode_chase_white(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, 255, 255, 255);
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, SEGMENT.colors[0]);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, SEGMENT.colors[0]);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, SEGMENT.colors[0]);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * White running on _color.
 */
uint16_t WS2812FX::mode_chase_color(void) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, 255, 255, 255);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, 255, 255, 255);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 255, 255, 255);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 255, 255, 255);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * White running followed by random color.
 */
uint16_t WS2812FX::mode_chase_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop, color_wheel(SEGMENT_RUNTIME.aux_param));
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }

  for(uint16_t i=0; i < SEGMENT_RUNTIME.counter_mode_step; i++) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 255, 255, 255);
  Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 255, 255, 255);

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * White running on rainbow.
 */
uint16_t WS2812FX::mode_chase_rainbow(void) {
  uint8_t color_sep = 256 / SEGMENT_LENGTH;
  uint8_t color_index = SEGMENT_RUNTIME.counter_mode_call & 0xFF;
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel(((i * color_sep) + color_index) & 0xFF));
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, 255, 255, 255);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, 255, 255, 255);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 255, 255, 255);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 255, 255, 255);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * White flashes running on _color.
 */
uint16_t WS2812FX::mode_chase_flash(void) {
  const static uint8_t flash_count = 4;
  uint8_t flash_step = SEGMENT_RUNTIME.counter_mode_call % ((flash_count * 2) + 1);

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, SEGMENT.colors[0]);
  }

//uint16 delay = (SEGMENT.speed / SEGMENT_LENGTH);
  uint16 delay = 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    if(flash_step % 2 == 0) {
      uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
      uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, 255, 255, 255);
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, 255, 255, 255);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 255, 255, 255);
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 255, 255, 255);
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
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel(SEGMENT_RUNTIME.aux_param));
  }

//uint16 delay = (SEGMENT.speed / SEGMENT_LENGTH);
  uint16 delay = 1 + ((10 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
  if(flash_step < (flash_count * 2)) {
    uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
    uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
    if(flash_step % 2 == 0) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 255, 255, 255);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 255, 255, 255);
      delay = 20;
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, color_wheel(SEGMENT_RUNTIME.aux_param));
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 0, 0, 0);
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
 * color chase function.
 * color1 = background color
 * color2 and color3 = colors of two adjacent leds
 */
uint16_t WS2812FX::chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    Adafruit_NeoPixel::setPixelColor(i, color1);
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, color2);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, color3);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, color2);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, color3);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Rainbow running on white.
 */
uint16_t WS2812FX::mode_chase_rainbow_white(void) {
  uint32_t color1 = 0xFFFFFF;
  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  uint32_t color2 = color_wheel(((n * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);
  uint32_t color3 = color_wheel(((m * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call & 0xFF)) & 0xFF);

  return chase(color1, color2, color3);
}


/*
 * Black running on _color.
 */
uint16_t WS2812FX::mode_chase_blackout(void) {
  return chase(SEGMENT.colors[0], 0, 0);
}


/*
 * Black running on rainbow.
 */
uint16_t WS2812FX::mode_chase_blackout_rainbow(void) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color_wheel(((i * 256 / SEGMENT_LENGTH) + (SEGMENT_RUNTIME.counter_mode_call % 256)) % 256));
  }

  uint16_t n = SEGMENT_RUNTIME.counter_mode_step;
  uint16_t m = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - n, 0, 0, 0);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - m, 0, 0, 0);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + n, 0, 0, 0);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + m, 0, 0, 0);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Random color intruduced alternating from start and end of strip.
 */
uint16_t WS2812FX::mode_color_sweep_random(void) {
  if(SEGMENT_RUNTIME.counter_mode_step == 0 || SEGMENT_RUNTIME.counter_mode_step == SEGMENT_LENGTH) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
  }

  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, color_wheel(SEGMENT_RUNTIME.aux_param));
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, color_wheel(SEGMENT_RUNTIME.aux_param));
    }
  } else {
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) + 1, color_wheel(SEGMENT_RUNTIME.aux_param));
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) - 1, color_wheel(SEGMENT_RUNTIME.aux_param));
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % (SEGMENT_LENGTH * 2);
//return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Alternating pixels running function.
 */
uint16_t WS2812FX::running(uint32_t color1, uint32_t color2) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i + SEGMENT_RUNTIME.counter_mode_step) % 4 < 2) {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color1);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color1);
      }
    } else {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color2);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color2);
      }
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 4;
//return (SEGMENT.speed / 4);
  return 10 + ((30 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Alternating color/white pixels running.
 */
uint16_t WS2812FX::mode_running_color(void) {
  return running(SEGMENT.colors[0], 0xFFFFFF);
}


/*
 * Alternating red/blue pixels running.
 */
uint16_t WS2812FX::mode_running_red_blue(void) {
  return running(0xFF0000, 0x0000FF);
}


/*
 * Random colored pixels running.
 */
uint16_t WS2812FX::mode_running_random(void) {
  for(uint16_t i=SEGMENT_LENGTH-1; i > 0; i--) {
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, Adafruit_NeoPixel::getPixelColor(SEGMENT.stop - i + 1));
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, Adafruit_NeoPixel::getPixelColor(SEGMENT.start + i - 1));
    }
  }

  if(SEGMENT_RUNTIME.counter_mode_step == 0) {
    SEGMENT_RUNTIME.aux_param = get_random_wheel_index(SEGMENT_RUNTIME.aux_param);
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop, color_wheel(SEGMENT_RUNTIME.aux_param));
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start, color_wheel(SEGMENT_RUNTIME.aux_param));
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 2;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 50 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * K.I.T.T.
 */
uint16_t WS2812FX::mode_larson_scanner(void) {

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    uint32_t px_rgb = Adafruit_NeoPixel::getPixelColor(i);

    byte px_r = (px_rgb & 0xFF0000) >> 16;
    byte px_g = (px_rgb & 0x00FF00) >>  8;
    byte px_b = (px_rgb & 0x0000FF);

    px_r = px_r >> 1; // fade out (divide by 2)
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    Adafruit_NeoPixel::setPixelColor(i, px_r, px_g, px_b);
  }

  if(SEGMENT_RUNTIME.counter_mode_step < SEGMENT_LENGTH) {
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    }
  } else {
    if(SEGMENT.reverse) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) + 2, SEGMENT.colors[0]);
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + ((SEGMENT_LENGTH * 2) - SEGMENT_RUNTIME.counter_mode_step) - 2, SEGMENT.colors[0]);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % ((SEGMENT_LENGTH * 2) - 2);
//return (SEGMENT.speed / (SEGMENT_LENGTH * 2));
  return 10 + ((10 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Firing comets from one end.
 */
uint16_t WS2812FX::mode_comet(void) {

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    uint32_t px_rgb = Adafruit_NeoPixel::getPixelColor(i);

    byte px_r = (px_rgb & 0xFF0000) >> 16;
    byte px_g = (px_rgb & 0x00FF00) >>  8;
    byte px_b = (px_rgb & 0x0000FF);

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    Adafruit_NeoPixel::setPixelColor(i, px_r, px_g, px_b);
  }

  if(SEGMENT.reverse) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  } else {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % SEGMENT_LENGTH;
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 10 + ((10 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Fireworks function.
 */
uint16_t WS2812FX::fireworks(uint32_t color) {
  uint32_t px_rgb = 0;
  byte px_r = 0;
  byte px_g = 0;
  byte px_b = 0;

  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    px_rgb = Adafruit_NeoPixel::getPixelColor(i);

    px_r = (px_rgb & 0x00FF0000) >> 16;
    px_g = (px_rgb & 0x0000FF00) >>  8;
    px_b = (px_rgb & 0x000000FF);

    // fade out (divide by 2)
    px_r = px_r >> 1;
    px_g = px_g >> 1;
    px_b = px_b >> 1;

    Adafruit_NeoPixel::setPixelColor(i, px_r, px_g, px_b);
  }

  // first LED has only one neighbour
  px_r = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.start+1) & 0xFF0000) >> 16) >> 1) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.start) & 0xFF0000) >> 16);
  px_g = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.start+1) & 0x00FF00) >>  8) >> 1) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.start) & 0x00FF00) >>  8);
  px_b = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.start+1) & 0x0000FF)      ) >> 1) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.start) & 0x0000FF));
  Adafruit_NeoPixel::setPixelColor(SEGMENT.start, px_r, px_g, px_b);

  // set brightness(i) = ((brightness(i-1)/2 + brightness(i+1)) / 2) + brightness(i)
  for(uint16_t i=SEGMENT.start+1; i <= SEGMENT.stop-1; i++) {
    px_r = ((
            (((Adafruit_NeoPixel::getPixelColor(i-1) & 0xFF0000) >> 16) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i+1) & 0xFF0000) >> 16)     ) ) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i  ) & 0xFF0000) >> 16)     );

    px_g = ((
            (((Adafruit_NeoPixel::getPixelColor(i-1) & 0x00FF00) >> 8) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i+1) & 0x00FF00) >> 8)     ) ) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i  ) & 0x00FF00) >> 8)     );

    px_b = ((
            (((Adafruit_NeoPixel::getPixelColor(i-1) & 0x0000FF)     ) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i+1) & 0x0000FF)     )     ) ) >> 1) +
            (((Adafruit_NeoPixel::getPixelColor(i  ) & 0x0000FF)     )     );

    Adafruit_NeoPixel::setPixelColor(i, px_r, px_g, px_b);
  }

  // last LED has only one neighbour
  px_r = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop-1) & 0xFF0000) >> 16) >> 2) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop) & 0xFF0000) >> 16);
  px_g = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop-1) & 0x00FF00) >>  8) >> 2) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop) & 0x00FF00) >>  8);
  px_b = (((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop-1) & 0x0000FF)      ) >> 2) + ((Adafruit_NeoPixel::getPixelColor(SEGMENT.stop) & 0x0000FF));
  Adafruit_NeoPixel::setPixelColor(SEGMENT.stop, px_r, px_g, px_b);

  if(!_triggered) {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/20); i++) {
      if(random(10) == 0) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
      }
    }
  } else {
    for(uint16_t i=0; i<max(1, SEGMENT_LENGTH/10); i++) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + random(SEGMENT_LENGTH), color);
    }
  }
//return (SEGMENT.speed / 8);
  return 20 + ((20 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Firework sparks.
 */
uint16_t WS2812FX::mode_fireworks(void) {
  uint32_t color = SEGMENT.colors[0];
  return fireworks(color);
}


/*
 * Random colored firework sparks.
 */
uint16_t WS2812FX::mode_fireworks_random(void) {
  uint32_t color = color_wheel(random(256));
  return fireworks(color);
}


/*
 * Alternating red/green pixels running.
 */
uint16_t WS2812FX::mode_merry_christmas(void) {
  return running(0xFF0000, 0x00FF00);
}

/*
 * Alternating orange/purple pixels running.
 */
uint16_t WS2812FX::mode_halloween(void) {
  return running(0xFF0082, 0xFF3200);
}


/*
 * Random flickering.
 */
uint16_t WS2812FX::mode_fire_flicker(void) {
  return fire_flicker(3);
}

/*
* Random flickering, less intesity.
*/
uint16_t WS2812FX::mode_fire_flicker_soft(void) {
  return fire_flicker(6);
}

/*
* Random flickering, more intesity.
*/
uint16_t WS2812FX::mode_fire_flicker_intense(void) {
  return fire_flicker(1.7);
}


/*
 * Fire flicker function
 */
uint16_t WS2812FX::fire_flicker(int rev_intensity) {
  byte p_r = (SEGMENT.colors[0] & 0xFF0000) >> 16;
  byte p_g = (SEGMENT.colors[0] & 0x00FF00) >>  8;
  byte p_b = (SEGMENT.colors[0] & 0x0000FF);
  byte flicker_val = max(p_r, max(p_g, p_b)) / rev_intensity;
  for(uint16_t i=SEGMENT.start; i <= SEGMENT.stop; i++) {
    int flicker = random(0, flicker_val);
    int r1 = p_r - flicker;
    int g1 = p_g - flicker;
    int b1 = p_b - flicker;
    if(g1<0) g1 = 0;
    if(r1<0) r1 = 0;
    if(b1<0) b1 = 0;
    Adafruit_NeoPixel::setPixelColor(i, r1, g1, b1);
  }
//return (SEGMENT.speed);
  return 10 + ((500 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SPEED_MAX);
}



/*
 * Lights all LEDs after each other up starting from the outer edges and
 * finishing in the middle. Then turns them in reverse order off. Repeat.
 */
uint16_t WS2812FX::mode_dual_color_wipe_in_out(void) {
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1;
  bool odd = (SEGMENT_LENGTH % 2) == 1;
  int mid = odd ? ((SEGMENT_LENGTH / 2) + 1) : (SEGMENT_LENGTH / 2);
  if (SEGMENT_RUNTIME.counter_mode_step < mid) {
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    Adafruit_NeoPixel::setPixelColor(SEGMENT.start + end, SEGMENT.colors[0]);
  } else {
    if (odd) {
      // If odd, we need to 'double count' the center LED (once to turn it on,
      // once to turn it off). So trail one behind after the middle LED.
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + end + 1, 0);
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + end, 0);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  }
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Lights all LEDs after each other up starting from the outer edges and
 * finishing in the middle. Then turns them in that order off. Repeat.
 */
uint16_t WS2812FX::mode_dual_color_wipe_in_in(void) {
  bool odd = (SEGMENT_LENGTH % 2) == 1;
  int mid = SEGMENT_LENGTH / 2;
  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);
    } else {
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i - 1, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, 0);
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);
    } else {
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, 0);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  }
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Lights all LEDs after each other up starting from the middle and
 * finishing at the edges. Then turns them off in that order. Repeat.
 */
uint16_t WS2812FX::mode_dual_color_wipe_out_out(void) {
  int end = SEGMENT_LENGTH - SEGMENT_RUNTIME.counter_mode_step - 1;
  bool odd = (SEGMENT_LENGTH % 2) == 1;
  int mid = SEGMENT_LENGTH / 2;

  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step - 1, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + end + 1, 0);
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_RUNTIME.counter_mode_step, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + end, 0);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  }
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Lights all LEDs after each other up starting from the middle and
 * finishing at the edges. Then turns them off in reverse order. Repeat.
 */
uint16_t WS2812FX::mode_dual_color_wipe_out_in(void) {
  bool odd = (SEGMENT_LENGTH % 2) == 1;
  int mid = SEGMENT_LENGTH / 2;

  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step <= mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i - 1, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i, 0);
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step < mid) {
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid - SEGMENT_RUNTIME.counter_mode_step - 1, SEGMENT.colors[0]);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + mid + SEGMENT_RUNTIME.counter_mode_step, SEGMENT.colors[0]);
    } else {
      int i = SEGMENT_RUNTIME.counter_mode_step - mid;
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, 0);
      Adafruit_NeoPixel::setPixelColor(SEGMENT.start + SEGMENT_LENGTH - i - 1, 0);
    }
  }

  SEGMENT_RUNTIME.counter_mode_step++;
  if (odd) {
    if (SEGMENT_RUNTIME.counter_mode_step > SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  } else {
    if (SEGMENT_RUNTIME.counter_mode_step >= SEGMENT_LENGTH) {
      SEGMENT_RUNTIME.counter_mode_step = 0;
    }
  }
//return (SEGMENT.speed / SEGMENT_LENGTH);
  return 5 + ((50 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}


/*
 * Alternating white/red/black pixels running.
 */
uint16_t WS2812FX::mode_circus_combustus(void) {
  return tricolor_chase(0xFF0000, 0xFFFFFF, 0);
}


/*
 * Tricolor chase function
 */
uint16_t WS2812FX::tricolor_chase(uint32_t color1, uint32_t color2, uint32_t color3) {
  for(uint16_t i=0; i < SEGMENT_LENGTH; i++) {
    if((i + SEGMENT_RUNTIME.counter_mode_step) % 6 < 2) {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color1);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color1);
      }
    } else if((i + SEGMENT_RUNTIME.counter_mode_step) % 6 < 4) {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color2);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color2);
      }
    } else {
      if(SEGMENT.reverse) {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.start + i, color3);
      } else {
        Adafruit_NeoPixel::setPixelColor(SEGMENT.stop - i, color3);
      }
    }
  }

  SEGMENT_RUNTIME.counter_mode_step = (SEGMENT_RUNTIME.counter_mode_step + 1) % 6;
//return (SEGMENT.speed / 6);
  return 100 + ((100 * (uint32_t)(SPEED_MAX - SEGMENT.speed)) / SEGMENT_LENGTH);
}

/*
 * Bicolor chase mode
 */
 uint16_t WS2812FX::mode_bicolor_chase(void) {
  return chase(SEGMENT.colors[0], SEGMENT.colors[1], SEGMENT.colors[2]);
}


/*
 * Tricolor chase mode
 */
 uint16_t WS2812FX::mode_tricolor_chase(void) {
  return tricolor_chase(SEGMENT.colors[0], SEGMENT.colors[1], SEGMENT.colors[2]);
}
