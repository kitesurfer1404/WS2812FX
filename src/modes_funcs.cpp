/*
  modes_funcs.cpp - WS2812FX effects helper functions

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

  return (_seg->speed / 16);
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
  return (_seg->speed / 16);
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
  return (_seg->speed / 16);
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

  return (_seg->speed / 16);
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
