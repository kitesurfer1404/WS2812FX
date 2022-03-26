/*
  modes_arduino.h - WS2812FX header file for Arduino microprocessors

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
#ifndef mode_arduino_h
#define mode_arduino_h

#define MODE_COUNT (sizeof(_names)/sizeof(_names[0]))
#define MODE_PTR(x) this->*_modes[x]
#define MODE_NAME(x) _names[x]

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_INV           4 
#define FX_MODE_COLOR_WIPE_REV           5
#define FX_MODE_COLOR_WIPE_REV_INV       6
#define FX_MODE_COLOR_WIPE_RANDOM        7
#define FX_MODE_RANDOM_COLOR             8
#define FX_MODE_SINGLE_DYNAMIC           9
#define FX_MODE_MULTI_DYNAMIC           10
#define FX_MODE_RAINBOW                 11
#define FX_MODE_RAINBOW_CYCLE           12
#define FX_MODE_SCAN                    13
#define FX_MODE_DUAL_SCAN               14
#define FX_MODE_FADE                    15
#define FX_MODE_THEATER_CHASE           16
#define FX_MODE_THEATER_CHASE_RAINBOW   17
#define FX_MODE_RUNNING_LIGHTS          18
#define FX_MODE_TWINKLE                 19
#define FX_MODE_TWINKLE_RANDOM          20
#define FX_MODE_TWINKLE_FADE            21
#define FX_MODE_TWINKLE_FADE_RANDOM     22
#define FX_MODE_SPARKLE                 23
#define FX_MODE_FLASH_SPARKLE           24
#define FX_MODE_HYPER_SPARKLE           25
#define FX_MODE_STROBE                  26
#define FX_MODE_STROBE_RAINBOW          27
#define FX_MODE_MULTI_STROBE            28
#define FX_MODE_BLINK_RAINBOW           29
#define FX_MODE_CHASE_WHITE             30
#define FX_MODE_CHASE_COLOR             31
#define FX_MODE_CHASE_RANDOM            32
#define FX_MODE_CHASE_RAINBOW           33
#define FX_MODE_CHASE_FLASH             34
#define FX_MODE_CHASE_FLASH_RANDOM      35
#define FX_MODE_CHASE_RAINBOW_WHITE     36
#define FX_MODE_CHASE_BLACKOUT          37
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  38
#define FX_MODE_COLOR_SWEEP_RANDOM      39
#define FX_MODE_RUNNING_COLOR           40
#define FX_MODE_RUNNING_RED_BLUE        41
#define FX_MODE_RUNNING_RANDOM          42
#define FX_MODE_LARSON_SCANNER          43
#define FX_MODE_COMET                   44
#define FX_MODE_FIREWORKS               45
#define FX_MODE_FIREWORKS_RANDOM        46
#define FX_MODE_MERRY_CHRISTMAS         47
#define FX_MODE_FIRE_FLICKER            48
#define FX_MODE_FIRE_FLICKER_SOFT       49
#define FX_MODE_FIRE_FLICKER_INTENSE    50
#define FX_MODE_CIRCUS_COMBUSTUS        51
#define FX_MODE_HALLOWEEN               52
#define FX_MODE_BICOLOR_CHASE           53
#define FX_MODE_TRICOLOR_CHASE          54
#define FX_MODE_TWINKLEFOX              55
#define FX_MODE_RAIN                    56
#define FX_MODE_CUSTOM                  57  // keep this for backward compatiblity
#define FX_MODE_CUSTOM_0                57  // custom modes need to go at the end
#define FX_MODE_CUSTOM_1                58
#define FX_MODE_CUSTOM_2                59
#define FX_MODE_CUSTOM_3                60
#define FX_MODE_CUSTOM_4                61
#define FX_MODE_CUSTOM_5                62
#define FX_MODE_CUSTOM_6                63
#define FX_MODE_CUSTOM_7                64

// create GLOBAL names to allow WS2812FX to compile with sketches and other libs
// that store strings in PROGMEM (get rid of the "section type conflict with __c"
// errors once and for all. Amen.)
const char name_0[] PROGMEM = "Static";
const char name_1[] PROGMEM = "Blink";
const char name_2[] PROGMEM = "Breath";
const char name_3[] PROGMEM = "Color Wipe";
const char name_4[] PROGMEM = "Color Wipe Inverse";
const char name_5[] PROGMEM = "Color Wipe Reverse";
const char name_6[] PROGMEM = "Color Wipe Reverse Inverse";
const char name_7[] PROGMEM = "Color Wipe Random";
const char name_8[] PROGMEM = "Random Color";
const char name_9[] PROGMEM = "Single Dynamic";
const char name_10[] PROGMEM = "Multi Dynamic";
const char name_11[] PROGMEM = "Rainbow";
const char name_12[] PROGMEM = "Rainbow Cycle";
const char name_13[] PROGMEM = "Scan";
const char name_14[] PROGMEM = "Dual Scan";
const char name_15[] PROGMEM = "Fade";
const char name_16[] PROGMEM = "Theater Chase";
const char name_17[] PROGMEM = "Theater Chase Rainbow";
const char name_18[] PROGMEM = "Running Lights";
const char name_19[] PROGMEM = "Twinkle";
const char name_20[] PROGMEM = "Twinkle Random";
const char name_21[] PROGMEM = "Twinkle Fade";
const char name_22[] PROGMEM = "Twinkle Fade Random";
const char name_23[] PROGMEM = "Sparkle";
const char name_24[] PROGMEM = "Flash Sparkle";
const char name_25[] PROGMEM = "Hyper Sparkle";
const char name_26[] PROGMEM = "Strobe";
const char name_27[] PROGMEM = "Strobe Rainbow";
const char name_28[] PROGMEM = "Multi Strobe";
const char name_29[] PROGMEM = "Blink Rainbow";
const char name_30[] PROGMEM = "Chase White";
const char name_31[] PROGMEM = "Chase Color";
const char name_32[] PROGMEM = "Chase Random";
const char name_33[] PROGMEM = "Chase Rainbow";
const char name_34[] PROGMEM = "Chase Flash";
const char name_35[] PROGMEM = "Chase Flash Random";
const char name_36[] PROGMEM = "Chase Rainbow White";
const char name_37[] PROGMEM = "Chase Blackout";
const char name_38[] PROGMEM = "Chase Blackout Rainbow";
const char name_39[] PROGMEM = "Color Sweep Random";
const char name_40[] PROGMEM = "Running Color";
const char name_41[] PROGMEM = "Running Red Blue";
const char name_42[] PROGMEM = "Running Random";
const char name_43[] PROGMEM = "Larson Scanner";
const char name_44[] PROGMEM = "Comet";
const char name_45[] PROGMEM = "Fireworks";
const char name_46[] PROGMEM = "Fireworks Random";
const char name_47[] PROGMEM = "Merry Christmas";
const char name_48[] PROGMEM = "Fire Flicker";
const char name_49[] PROGMEM = "Fire Flicker (soft)";
const char name_50[] PROGMEM = "Fire Flicker (intense)";
const char name_51[] PROGMEM = "Circus Combustus";
const char name_52[] PROGMEM = "Halloween";
const char name_53[] PROGMEM = "Bicolor Chase";
const char name_54[] PROGMEM = "Tricolor Chase";
const char name_55[] PROGMEM = "TwinkleFOX";
const char name_56[] PROGMEM = "Rain";
const char name_57[] PROGMEM = "Custom 0"; // custom modes need to go at the end
const char name_58[] PROGMEM = "Custom 1";
const char name_59[] PROGMEM = "Custom 2";
const char name_60[] PROGMEM = "Custom 3";
const char name_61[] PROGMEM = "Custom 4";
const char name_62[] PROGMEM = "Custom 5";
const char name_63[] PROGMEM = "Custom 6";
const char name_64[] PROGMEM = "Custom 7";

static const __FlashStringHelper* _names[] = {
  FSH(name_0),
  FSH(name_1),
  FSH(name_2),
  FSH(name_3),
  FSH(name_4),
  FSH(name_5),
  FSH(name_6),
  FSH(name_7),
  FSH(name_8),
  FSH(name_9),
  FSH(name_10),
  FSH(name_11),
  FSH(name_12),
  FSH(name_13),
  FSH(name_14),
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
  FSH(name_29),
  FSH(name_30),
  FSH(name_31),
  FSH(name_32),
  FSH(name_33),
  FSH(name_34),
  FSH(name_35),
  FSH(name_36),
  FSH(name_37),
  FSH(name_38),
  FSH(name_39),
  FSH(name_40),
  FSH(name_41),
  FSH(name_42),
  FSH(name_43),
  FSH(name_44),
  FSH(name_45),
  FSH(name_46),
  FSH(name_47),
  FSH(name_48),
  FSH(name_49),
  FSH(name_50),
  FSH(name_51),
  FSH(name_52),
  FSH(name_53),
  FSH(name_54),
  FSH(name_55),
  FSH(name_56),
  FSH(name_57),
  FSH(name_58),
  FSH(name_59),
  FSH(name_60),
  FSH(name_61),
  FSH(name_62),
  FSH(name_63),
  FSH(name_64)
};

// define static array of member function pointers.
// function pointers MUST be in the same order as the corresponding name in the _name array.
__attribute__ ((unused)) static WS2812FX::mode_ptr _modes[] = {
  &WS2812FX::mode_static,
  &WS2812FX::mode_blink,
  &WS2812FX::mode_breath,
  &WS2812FX::mode_color_wipe,
  &WS2812FX::mode_color_wipe_inv,
  &WS2812FX::mode_color_wipe_rev,
  &WS2812FX::mode_color_wipe_rev_inv,
  &WS2812FX::mode_color_wipe_random,
  &WS2812FX::mode_random_color,
  &WS2812FX::mode_single_dynamic,
  &WS2812FX::mode_multi_dynamic,
  &WS2812FX::mode_rainbow,
  &WS2812FX::mode_rainbow_cycle,
  &WS2812FX::mode_scan,
  &WS2812FX::mode_dual_scan,
  &WS2812FX::mode_fade,
  &WS2812FX::mode_theater_chase,
  &WS2812FX::mode_theater_chase_rainbow,
  &WS2812FX::mode_running_lights,
  &WS2812FX::mode_twinkle,
  &WS2812FX::mode_twinkle_random,
  &WS2812FX::mode_twinkle_fade,
  &WS2812FX::mode_twinkle_fade_random,
  &WS2812FX::mode_sparkle,
  &WS2812FX::mode_flash_sparkle,
  &WS2812FX::mode_hyper_sparkle,
  &WS2812FX::mode_strobe,
  &WS2812FX::mode_strobe_rainbow,
  &WS2812FX::mode_multi_strobe,
  &WS2812FX::mode_blink_rainbow,
  &WS2812FX::mode_chase_white,
  &WS2812FX::mode_chase_color,
  &WS2812FX::mode_chase_random,
  &WS2812FX::mode_chase_rainbow,
  &WS2812FX::mode_chase_flash,
  &WS2812FX::mode_chase_flash_random,
  &WS2812FX::mode_chase_rainbow_white,
  &WS2812FX::mode_chase_blackout,
  &WS2812FX::mode_chase_blackout_rainbow,
  &WS2812FX::mode_color_sweep_random,
  &WS2812FX::mode_running_color,
  &WS2812FX::mode_running_red_blue,
  &WS2812FX::mode_running_random,
  &WS2812FX::mode_larson_scanner,
  &WS2812FX::mode_comet,
  &WS2812FX::mode_fireworks,
  &WS2812FX::mode_fireworks_random,
  &WS2812FX::mode_merry_christmas,
  &WS2812FX::mode_fire_flicker,
  &WS2812FX::mode_fire_flicker_soft,
  &WS2812FX::mode_fire_flicker_intense,
  &WS2812FX::mode_circus_combustus,
  &WS2812FX::mode_halloween,
  &WS2812FX::mode_bicolor_chase,
  &WS2812FX::mode_tricolor_chase,
  &WS2812FX::mode_twinkleFOX,
  &WS2812FX::mode_rain,
  &WS2812FX::mode_custom_0,
  &WS2812FX::mode_custom_1,
  &WS2812FX::mode_custom_2,
  &WS2812FX::mode_custom_3,
  &WS2812FX::mode_custom_4,
  &WS2812FX::mode_custom_5,
  &WS2812FX::mode_custom_6,
  &WS2812FX::mode_custom_7
};
#endif
