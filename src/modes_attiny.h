/*
  modes_attiny.h - WS2812FX header file for ATtiny microprocessors

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

  2022-03-23   Separated from the original WS2812FX.h file
*/
#ifndef mode_attiny_h
#define mode_attiny_h

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))
#define MODE_PTR(x) this->*_modes[x]
#define MODE_NAME(x) _names[x]

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_STROBE                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_REV           4
#define FX_MODE_TRICOLOR_CHASE           5
#define FX_MODE_SPARKLE                  6

#if (PROGMEM_SIZE > 4096UL) // if ATtiny has more tha 4k flash memory, include more effects
#define FX_MODE_BREATH                   7
#define FX_MODE_BICOLOR_CHASE            8
#define FX_MODE_LARSON_SCANNER           9
#define FX_MODE_RAINBOW_CYCLE           10
#define FX_MODE_RANDOM_COLOR            11
#define FX_MODE_FADE                    12
#define FX_MODE_COLOR_WIPE_RANDOM       13
#define FX_MODE_COLOR_SWEEP_RANDOM      14
#endif

#if (PROGMEM_SIZE > 8192UL) // if ATtiny has more tha 8k flash memory, include more effects
#define FX_MODE_FIREWORKS               15
#define FX_MODE_FIREWORKS_RANDOM        16
#define FX_MODE_FIRE_FLICKER            17
#define FX_MODE_FIRE_FLICKER_SOFT       18
#define FX_MODE_FIRE_FLICKER_INTENSE    19
#define FX_MODE_RUNNING_COLOR           20
#define FX_MODE_RUNNING_LIGHTS          21
#define FX_MODE_RUNNING_RANDOM          22
#define FX_MODE_SINGLE_DYNAMIC          23
#define FX_MODE_MULTI_DYNAMIC           24
#define FX_MODE_TWINKLEFOX              25
#define FX_MODE_HEARTBEAT               26
#define FX_MODE_BLOCK_DISSOLVE          27
#define FX_MODE_ICU                     28
#endif

#if (PROGMEM_SIZE > 16384UL) // if ATtiny has more tha 16k flash memory, include more effects
#define FX_MODE_RAIN                    29
#define FX_MODE_POPCORN                 30
#define FX_MODE_OSCILLATOR              31
#endif

#define FX_MODE_CUSTOM_0                 0  // custom modes are not supported and will simply run the STATIC effect

// create GLOBAL names to allow WS2812FX to compile with sketches and other libs
// that store strings in PROGMEM (get rid of the "section type conflict with __c"
// errors once and for all. Amen.)
const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Strobe";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Reverse";
const char name_5[] PROGMEM = "Tricolor Chase";
const char name_6[] PROGMEM = "Sparkle";


#if (PROGMEM_SIZE > 4096UL) // if ATtiny has more tha 4k flash memory, include more effects
const char name_7[] PROGMEM = "Breath";
const char name_8[] PROGMEM = "Bicolor Chase";
const char name_9[] PROGMEM = "Larson Scanner";
const char name_10[] PROGMEM = "Rainbow Cycle";
const char name_11[] PROGMEM = "Random Color";
const char name_12[] PROGMEM = "Fade";
const char name_13[] PROGMEM = "Color Wipe Random";
const char name_14[] PROGMEM = "Color Sweep Random";
#endif

#if (PROGMEM_SIZE > 8192UL) // if ATtiny has more tha 8k flash memory, include more effects
const char name_15[] PROGMEM = "Fireworks";
const char name_16[] PROGMEM = "Fireworks Random";
const char name_17[] PROGMEM = "Fire Flicker";
const char name_18[] PROGMEM = "Fire Flicker Soft";
const char name_19[] PROGMEM = "Fire Flicker Intense";
const char name_20[] PROGMEM = "Running Color";
const char name_21[] PROGMEM = "Running Lights";
const char name_22[] PROGMEM = "Running Random";
const char name_23[] PROGMEM = "Single Dynamic";
const char name_24[] PROGMEM = "Multi Dynamic";
const char name_25[] PROGMEM = "TinkleFox";
const char name_26[] PROGMEM = "Heartbeat";
const char name_27[] PROGMEM = "Block Dissolve";
const char name_28[] PROGMEM = "ICU";
#endif

#if (PROGMEM_SIZE > 16384UL) // if ATtiny has more tha 16k flash memory, include more effects
const char name_29[] PROGMEM = "Rain";
const char name_30[] PROGMEM = "Popcorn";
const char name_31[] PROGMEM = "Oscillator";
#endif

const char name_32[] PROGMEM = "Custom 0"; // custom modes need to go at the end

 __attribute__ ((unused)) static const __FlashStringHelper* _names[] = {
  FSH(name_0),
  FSH(name_1),
  FSH(name_2),
  FSH(name_3),
  FSH(name_4),
  FSH(name_5),
  FSH(name_6),

#if (PROGMEM_SIZE > 4096UL) // if ATtiny has more tha 4k flash memory, include more effects
  FSH(name_7),
  FSH(name_8),
  FSH(name_9),
  FSH(name_10),
  FSH(name_11),
  FSH(name_12),
  FSH(name_13),
  FSH(name_14),
#endif

#if (PROGMEM_SIZE > 8192UL) // if ATtiny has more tha 8k flash memory, include more effects
  FSH(name_15),
  FSH(name_16),
  FSH(name_17),
  FSH(name_18),
  FSH(name_19),
  FSH(name_20),
  FSH(name_21),
  FSH(name_22),
  FSH(name_23),
  FSH(name_24),
  FSH(name_25),
  FSH(name_26),
  FSH(name_27),
  FSH(name_28),
#endif

#if (PROGMEM_SIZE > 16384UL) // if ATtiny has more tha 16k flash memory, include more effects
  FSH(name_29),
  FSH(name_30),
  FSH(name_31),
#endif

  FSH(name_32)
};

// define static array of member function pointers.
// function pointers MUST be in the same order as the corresponding name in the _name array.
__attribute__ ((unused)) static WS2812FX::mode_ptr _modes[] = {
  &WS2812FX::mode_static,
  &WS2812FX::mode_blink,
  &WS2812FX::mode_strobe,
  &WS2812FX::mode_color_wipe,
  &WS2812FX::mode_color_wipe_rev,
  &WS2812FX::mode_tricolor_chase,
  &WS2812FX::mode_sparkle,

#if (PROGMEM_SIZE > 4096UL) // if ATtiny has more tha 4k flash memory, include more effects
  &WS2812FX::mode_breath,
  &WS2812FX::mode_bicolor_chase,
  &WS2812FX::mode_larson_scanner,
  &WS2812FX::mode_rainbow_cycle,
  &WS2812FX::mode_random_color,
  &WS2812FX::mode_fade,
  &WS2812FX::mode_color_wipe_random,
  &WS2812FX::mode_color_sweep_random,
#endif

#if (PROGMEM_SIZE > 8192UL) // if ATtiny has more tha 8k flash memory, include more effects
  &WS2812FX::mode_fireworks,
  &WS2812FX::mode_fireworks_random,
  &WS2812FX::mode_fire_flicker,
  &WS2812FX::mode_fire_flicker_soft,
  &WS2812FX::mode_fire_flicker_intense,
  &WS2812FX::mode_running_color,
  &WS2812FX::mode_running_lights,
  &WS2812FX::mode_running_random,
  &WS2812FX::mode_single_dynamic,
  &WS2812FX::mode_multi_dynamic,
  &WS2812FX::mode_twinkleFOX,
  &WS2812FX::mode_heartbeat,
  &WS2812FX::mode_block_dissolve,
  &WS2812FX::mode_icu,
#endif

#if (PROGMEM_SIZE > 16384UL) // if ATtiny has more tha 16k flash memory, include more effects
  &WS2812FX::mode_rain,
  &WS2812FX::mode_popcorn,
  &WS2812FX::mode_oscillator,
#endif

  &WS2812FX::mode_custom_0
};
#endif
