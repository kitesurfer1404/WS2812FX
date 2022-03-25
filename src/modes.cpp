/*
  modes.cpp - WS2812FX animation modes/effects

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

  2022-03-23   Separated from the original WS2812FX.cpp file
*/

#include "WS2812FX.h"

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
  return (_seg->speed / 16) ;
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
  return (_seg->speed / 4);
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
  uint32_t color = color_wheel(_seg_rt->counter_mode_step);
  if(IS_REVERSE) {
    copyPixels(_seg->start, _seg->start + 1, _seg_len - 1);
    setPixelColor(_seg->stop, color);
  } else {
    copyPixels(_seg->start + 1, _seg->start, _seg_len - 1);
    setPixelColor(_seg->start, color);
  }

  _seg_rt->counter_mode_step = (_seg_rt->counter_mode_step + 1) & 0xFF;
  if(_seg_rt->counter_mode_step == 0) SET_CYCLE;

  return (_seg->speed / 256);
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
  SET_CYCLE;
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

// block dissolve effect
uint16_t WS2812FX::mode_block_dissolve(void) {
  uint32_t color = _seg->colors[_seg_rt->aux_param]; // get the target color

  // get the decimated color after setPixelColor() has mangled it
  // in accordance to the brightness setting
  setPixelColor(_seg->start, color);
  uint32_t desColor = getPixelColor(_seg->start);

  // find a random pixel that isn't the target color and update it
  for(uint16_t i=0; i<_seg_len; i++) {
    int index = _seg->start + random16(_seg_len);
    if(getPixelColor(index) != desColor) {
      setPixelColor(index, color);
      return _seg->speed / 64;
    }
  }

  // if didn't find a random pixel that wasn't the target color,
  // then set the entire segment to the target color
  fill(color, _seg->start, _seg_len);

  // choose a new target color
  _seg_rt->aux_param = (_seg_rt->aux_param + 1) % MAX_NUM_COLORS;
  if(_seg_rt->aux_param == 0) SET_CYCLE;
  return _seg->speed / 64;
}

// ICU effect
uint16_t WS2812FX::mode_icu(void) {
  uint16_t pos = _seg_rt->counter_mode_step; // current eye position
  uint16_t dest = _seg_rt->aux_param3;       // eye destination
  uint16_t index = _seg->start + pos;        // index of the first eye
  uint16_t index2 = index + _seg_len/2;      // index of the second eye

  Adafruit_NeoPixel::clear(); // erase the current eyes

  // if the eyes have not reached their destination
  if(pos != dest) {
    // move the eyes right or left depending on position relative to destination
    int dir = dest > pos ? 1 : -1;
    setPixelColor(index + dir, _seg->colors[0]); // paint two eyes
    setPixelColor(index2 + dir, _seg->colors[0]);
    _seg_rt->counter_mode_step += dir; // update the eye position
    return (_seg->speed / _seg_len);
  } else { // the eyes have reached their destination
    if(random8(6) == 0) {  // blink the eyes once in a while
      return 200;
    } else {
      setPixelColor(index, _seg->colors[0]);
      setPixelColor(index2, _seg->colors[0]);
      _seg_rt->aux_param3 = random16(_seg_len/2); // set a new destination
      SET_CYCLE;
      return 1000 + random16(2000); // pause a second or two
    }
  }
}

// Dual Larson effect
uint16_t WS2812FX::mode_dual_larson(void) {
  fade_out();

  _seg_rt->aux_param3 += _seg_rt->aux_param ? -1 : 1; // update the LED index

  setPixelColor(_seg->start + _seg_rt->aux_param3, _seg->colors[0]);
  setPixelColor(_seg->stop  - _seg_rt->aux_param3, _seg->colors[2] ? _seg->colors[2] : _seg->colors[0]);

  if(_seg_rt->aux_param3 == 0 || _seg_rt->aux_param3 >= _seg_len - 1) {
    _seg_rt->aux_param = !_seg_rt->aux_param; // change direction
    SET_CYCLE;
  }

  return (_seg->speed / (_seg_len * 2));
}

// Random Wipe Bright effect (same as custom RandomChase effect)
uint16_t WS2812FX::mode_random_wipe_bright(void) {
  uint32_t color = IS_REVERSE ? getPixelColor(_seg->stop): getPixelColor(_seg->start);

  // periodically change each RGB component to a random value
  uint8_t mask = random8(7);
  color = mask & 0x1 ? color : (color & 0x00ffff) | (random8() << 16);
  color = mask & 0x2 ? color : (color & 0xff00ff) | (random8() <<  8);
  color = mask & 0x4 ? color : (color & 0xffff00) | (random8());

  return running(color, color);
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
