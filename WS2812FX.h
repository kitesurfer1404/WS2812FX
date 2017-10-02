/*
  WS2812FX.h - Library for WS2812 LED effects.

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
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

#define DEFAULT_BRIGHTNESS 50
#define DEFAULT_MODE 0
#define DEFAULT_SPEED 150
#define DEFAULT_COLOR 0xFF0000

#define SPEED_MIN 0
#define SPEED_MAX 255

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255
      
#define MAX_NUM_SEGMENTS 10
#define SEGMENT          _segments[_segment_index]
#define SEGMENT_RUNTIME  _segment_runtimes[_segment_index]
#define SEGMENT_LENGTH   (SEGMENT.stop - SEGMENT.start + 1)
#define RESET_RUNTIME    memset(_segment_runtimes, 0, sizeof(_segment_runtimes))

#define MODE_COUNT 57

#define FX_MODE_STATIC                   0
#define FX_MODE_BLINK                    1
#define FX_MODE_BREATH                   2
#define FX_MODE_COLOR_WIPE               3
#define FX_MODE_COLOR_WIPE_RANDOM        4
#define FX_MODE_RANDOM_COLOR             5
#define FX_MODE_SINGLE_DYNAMIC           6
#define FX_MODE_MULTI_DYNAMIC            7
#define FX_MODE_RAINBOW                  8
#define FX_MODE_RAINBOW_CYCLE            9
#define FX_MODE_SCAN                    10
#define FX_MODE_DUAL_SCAN               11
#define FX_MODE_FADE                    12
#define FX_MODE_THEATER_CHASE           13
#define FX_MODE_THEATER_CHASE_RAINBOW   14
#define FX_MODE_RUNNING_LIGHTS          15
#define FX_MODE_TWINKLE                 16
#define FX_MODE_TWINKLE_RANDOM          17
#define FX_MODE_TWINKLE_FADE            18
#define FX_MODE_TWINKLE_FADE_RANDOM     19
#define FX_MODE_SPARKLE                 20
#define FX_MODE_FLASH_SPARKLE           21
#define FX_MODE_HYPER_SPARKLE           22
#define FX_MODE_STROBE                  23
#define FX_MODE_STROBE_RAINBOW          24
#define FX_MODE_MULTI_STROBE            25
#define FX_MODE_BLINK_RAINBOW           26
#define FX_MODE_CHASE_WHITE             27
#define FX_MODE_CHASE_COLOR             28
#define FX_MODE_CHASE_RANDOM            29
#define FX_MODE_CHASE_RAINBOW           30
#define FX_MODE_CHASE_FLASH             31
#define FX_MODE_CHASE_FLASH_RANDOM      32
#define FX_MODE_CHASE_RAINBOW_WHITE     33
#define FX_MODE_CHASE_BLACKOUT          34
#define FX_MODE_CHASE_BLACKOUT_RAINBOW  35
#define FX_MODE_COLOR_SWEEP_RANDOM      36
#define FX_MODE_RUNNING_COLOR           37
#define FX_MODE_RUNNING_RED_BLUE        38
#define FX_MODE_RUNNING_RANDOM          39
#define FX_MODE_LARSON_SCANNER          40
#define FX_MODE_COMET                   41
#define FX_MODE_FIREWORKS               42
#define FX_MODE_FIREWORKS_RANDOM        43
#define FX_MODE_MERRY_CHRISTMAS         44
#define FX_MODE_FIRE_FLICKER            45
#define FX_MODE_FIRE_FLICKER_SOFT       46
#define FX_MODE_FIRE_FLICKER_INTENSE    47
#define FX_MODE_DUAL_COLOR_WIPE_IN_OUT  48
#define FX_MODE_DUAL_COLOR_WIPE_IN_IN   49
#define FX_MODE_DUAL_COLOR_WIPE_OUT_OUT 50
#define FX_MODE_DUAL_COLOR_WIPE_OUT_IN  51
#define FX_MODE_CIRCUS_COMBUSTUS        52
#define FX_MODE_HALLOWEEN               53
#define FX_MODE_COLOR_WIPE_INVERSE      54
#define FX_MODE_BICOLOR_CHASE           55
#define FX_MODE_TRICOLOR_CHASE          56

class WS2812FX : public Adafruit_NeoPixel {

  typedef uint16_t (WS2812FX::*mode_ptr)(void);
  
  // segment parameters
  typedef struct segment {
    uint8_t  mode;
    uint32_t color[3];
    uint16_t speed;
    uint16_t start;
    uint16_t stop;
    bool     reverse;
  } segment;

  // segment runtime parameters
  typedef struct segment_runtime {
    uint32_t counter_mode_step;
    uint32_t counter_mode_call;
    unsigned long next_time;
    uint8_t aux_param;
  } segment_runtime;

  public:

    WS2812FX(uint16_t n, uint8_t p, neoPixelType t) : Adafruit_NeoPixel(n, p, t) {
      _mode[FX_MODE_STATIC]                  = &WS2812FX::mode_static;
      _mode[FX_MODE_BLINK]                   = &WS2812FX::mode_blink;
      _mode[FX_MODE_BREATH]                  = &WS2812FX::mode_breath;
      _mode[FX_MODE_COLOR_WIPE]              = &WS2812FX::mode_color_wipe;
      _mode[FX_MODE_COLOR_WIPE_INVERSE]      = &WS2812FX::mode_color_wipe_inverse;
      _mode[FX_MODE_COLOR_WIPE_RANDOM]       = &WS2812FX::mode_color_wipe_random;
      _mode[FX_MODE_RANDOM_COLOR]            = &WS2812FX::mode_random_color;
      _mode[FX_MODE_SINGLE_DYNAMIC]          = &WS2812FX::mode_single_dynamic;
      _mode[FX_MODE_MULTI_DYNAMIC]           = &WS2812FX::mode_multi_dynamic;
      _mode[FX_MODE_RAINBOW]                 = &WS2812FX::mode_rainbow;
      _mode[FX_MODE_RAINBOW_CYCLE]           = &WS2812FX::mode_rainbow_cycle;
      _mode[FX_MODE_SCAN]                    = &WS2812FX::mode_scan;
      _mode[FX_MODE_DUAL_SCAN]               = &WS2812FX::mode_dual_scan;
      _mode[FX_MODE_FADE]                    = &WS2812FX::mode_fade;
      _mode[FX_MODE_THEATER_CHASE]           = &WS2812FX::mode_theater_chase;
      _mode[FX_MODE_THEATER_CHASE_RAINBOW]   = &WS2812FX::mode_theater_chase_rainbow;
      _mode[FX_MODE_RUNNING_LIGHTS]          = &WS2812FX::mode_running_lights;
      _mode[FX_MODE_TWINKLE]                 = &WS2812FX::mode_twinkle;
      _mode[FX_MODE_TWINKLE_RANDOM]          = &WS2812FX::mode_twinkle_random;
      _mode[FX_MODE_TWINKLE_FADE]            = &WS2812FX::mode_twinkle_fade;
      _mode[FX_MODE_TWINKLE_FADE_RANDOM]     = &WS2812FX::mode_twinkle_fade_random;
      _mode[FX_MODE_SPARKLE]                 = &WS2812FX::mode_sparkle;
      _mode[FX_MODE_FLASH_SPARKLE]           = &WS2812FX::mode_flash_sparkle;
      _mode[FX_MODE_HYPER_SPARKLE]           = &WS2812FX::mode_hyper_sparkle;
      _mode[FX_MODE_STROBE]                  = &WS2812FX::mode_strobe;
      _mode[FX_MODE_STROBE_RAINBOW]          = &WS2812FX::mode_strobe_rainbow;
      _mode[FX_MODE_MULTI_STROBE]            = &WS2812FX::mode_multi_strobe;
      _mode[FX_MODE_BLINK_RAINBOW]           = &WS2812FX::mode_blink_rainbow;
      _mode[FX_MODE_CHASE_WHITE]             = &WS2812FX::mode_chase_white;
      _mode[FX_MODE_CHASE_COLOR]             = &WS2812FX::mode_chase_color;
      _mode[FX_MODE_CHASE_RANDOM]            = &WS2812FX::mode_chase_random;
      _mode[FX_MODE_CHASE_RAINBOW]           = &WS2812FX::mode_chase_rainbow;
      _mode[FX_MODE_CHASE_FLASH]             = &WS2812FX::mode_chase_flash;
      _mode[FX_MODE_CHASE_FLASH_RANDOM]      = &WS2812FX::mode_chase_flash_random;
      _mode[FX_MODE_CHASE_RAINBOW_WHITE]     = &WS2812FX::mode_chase_rainbow_white;
      _mode[FX_MODE_CHASE_BLACKOUT]          = &WS2812FX::mode_chase_blackout;
      _mode[FX_MODE_CHASE_BLACKOUT_RAINBOW]  = &WS2812FX::mode_chase_blackout_rainbow;
      _mode[FX_MODE_COLOR_SWEEP_RANDOM]      = &WS2812FX::mode_color_sweep_random;
      _mode[FX_MODE_RUNNING_COLOR]           = &WS2812FX::mode_running_color;
      _mode[FX_MODE_RUNNING_RED_BLUE]        = &WS2812FX::mode_running_red_blue;
      _mode[FX_MODE_RUNNING_RANDOM]          = &WS2812FX::mode_running_random;
      _mode[FX_MODE_LARSON_SCANNER]          = &WS2812FX::mode_larson_scanner;
      _mode[FX_MODE_COMET]                   = &WS2812FX::mode_comet;
      _mode[FX_MODE_FIREWORKS]               = &WS2812FX::mode_fireworks;
      _mode[FX_MODE_FIREWORKS_RANDOM]        = &WS2812FX::mode_fireworks_random;
      _mode[FX_MODE_MERRY_CHRISTMAS]         = &WS2812FX::mode_merry_christmas;
      _mode[FX_MODE_HALLOWEEN]               = &WS2812FX::mode_halloween;
      _mode[FX_MODE_FIRE_FLICKER]            = &WS2812FX::mode_fire_flicker;
      _mode[FX_MODE_FIRE_FLICKER_SOFT]       = &WS2812FX::mode_fire_flicker_soft;
      _mode[FX_MODE_FIRE_FLICKER_INTENSE]    = &WS2812FX::mode_fire_flicker_intense;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_OUT]  = &WS2812FX::mode_dual_color_wipe_in_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_IN_IN]   = &WS2812FX::mode_dual_color_wipe_in_in;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_OUT] = &WS2812FX::mode_dual_color_wipe_out_out;
      _mode[FX_MODE_DUAL_COLOR_WIPE_OUT_IN]  = &WS2812FX::mode_dual_color_wipe_out_in;
      _mode[FX_MODE_CIRCUS_COMBUSTUS]        = &WS2812FX::mode_circus_combustus;
      _mode[FX_MODE_BICOLOR_CHASE]           = &WS2812FX::mode_bicolor_chase;
      _mode[FX_MODE_TRICOLOR_CHASE]          = &WS2812FX::mode_tricolor_chase;


      _name[FX_MODE_STATIC]                    = F("Static");
      _name[FX_MODE_BLINK]                     = F("Blink");
      _name[FX_MODE_BREATH]                    = F("Breath");
      _name[FX_MODE_COLOR_WIPE]                = F("Color Wipe");
      _name[FX_MODE_COLOR_WIPE_INVERSE]        = F("Color Wipe Inverse");
      _name[FX_MODE_COLOR_WIPE_RANDOM]         = F("Color Wipe Random");
      _name[FX_MODE_RANDOM_COLOR]              = F("Random Color");
      _name[FX_MODE_SINGLE_DYNAMIC]            = F("Single Dynamic");
      _name[FX_MODE_MULTI_DYNAMIC]             = F("Multi Dynamic");
      _name[FX_MODE_RAINBOW]                   = F("Rainbow");
      _name[FX_MODE_RAINBOW_CYCLE]             = F("Rainbow Cycle");
      _name[FX_MODE_SCAN]                      = F("Scan");
      _name[FX_MODE_DUAL_SCAN]                 = F("Dual Scan");
      _name[FX_MODE_FADE]                      = F("Fade");
      _name[FX_MODE_THEATER_CHASE]             = F("Theater Chase");
      _name[FX_MODE_THEATER_CHASE_RAINBOW]     = F("Theater Chase Rainbow");
      _name[FX_MODE_RUNNING_LIGHTS]            = F("Running Lights");
      _name[FX_MODE_TWINKLE]                   = F("Twinkle");
      _name[FX_MODE_TWINKLE_RANDOM]            = F("Twinkle Random");
      _name[FX_MODE_TWINKLE_FADE]              = F("Twinkle Fade");
      _name[FX_MODE_TWINKLE_FADE_RANDOM]       = F("Twinkle Fade Random");
      _name[FX_MODE_SPARKLE]                   = F("Sparkle");
      _name[FX_MODE_FLASH_SPARKLE]             = F("Flash Sparkle");
      _name[FX_MODE_HYPER_SPARKLE]             = F("Hyper Sparkle");
      _name[FX_MODE_STROBE]                    = F("Strobe");
      _name[FX_MODE_STROBE_RAINBOW]            = F("Strobe Rainbow");
      _name[FX_MODE_MULTI_STROBE]              = F("Multi Strobe");
      _name[FX_MODE_BLINK_RAINBOW]             = F("Blink Rainbow");
      _name[FX_MODE_CHASE_WHITE]               = F("Chase White");
      _name[FX_MODE_CHASE_COLOR]               = F("Chase Color");
      _name[FX_MODE_CHASE_RANDOM]              = F("Chase Random");
      _name[FX_MODE_CHASE_RAINBOW]             = F("Chase Rainbow");
      _name[FX_MODE_CHASE_FLASH]               = F("Chase Flash");
      _name[FX_MODE_CHASE_FLASH_RANDOM]        = F("Chase Flash Random");
      _name[FX_MODE_CHASE_RAINBOW_WHITE]       = F("Chase Rainbow White");
      _name[FX_MODE_CHASE_BLACKOUT]            = F("Chase Blackout");
      _name[FX_MODE_CHASE_BLACKOUT_RAINBOW]    = F("Chase Blackout Rainbow");
      _name[FX_MODE_COLOR_SWEEP_RANDOM]        = F("Color Sweep Random");
      _name[FX_MODE_RUNNING_COLOR]             = F("Running Color");
      _name[FX_MODE_RUNNING_RED_BLUE]          = F("Running Red Blue");
      _name[FX_MODE_RUNNING_RANDOM]            = F("Running Random");
      _name[FX_MODE_LARSON_SCANNER]            = F("Larson Scanner");
      _name[FX_MODE_COMET]                     = F("Comet");
      _name[FX_MODE_FIREWORKS]                 = F("Fireworks");
      _name[FX_MODE_FIREWORKS_RANDOM]          = F("Fireworks Random");
      _name[FX_MODE_MERRY_CHRISTMAS]           = F("Merry Christmas");
      _name[FX_MODE_HALLOWEEN]                 = F("Halloween");
      _name[FX_MODE_FIRE_FLICKER]              = F("Fire Flicker");
      _name[FX_MODE_FIRE_FLICKER_SOFT]         = F("Fire Flicker (soft)");
      _name[FX_MODE_FIRE_FLICKER_INTENSE]      = F("Fire Flicker (intense)");
      _name[FX_MODE_DUAL_COLOR_WIPE_IN_OUT]    = F("Dual Color Wipe In to Out");
      _name[FX_MODE_DUAL_COLOR_WIPE_IN_IN]     = F("Dual Color Wipe In to In");
      _name[FX_MODE_DUAL_COLOR_WIPE_OUT_OUT]   = F("Dual Color Wipe Out to Out");
      _name[FX_MODE_DUAL_COLOR_WIPE_OUT_IN]    = F("Dual Color Wipe Out to In");
      _name[FX_MODE_CIRCUS_COMBUSTUS]          = F("Circus Combustus");
      _name[FX_MODE_BICOLOR_CHASE]             = F("Bicolor Chase");
      _name[FX_MODE_TRICOLOR_CHASE]            = F("Tricolor Chase");


//      _mode_index = DEFAULT_MODE;
//      _speed = DEFAULT_SPEED;
      _brightness = DEFAULT_BRIGHTNESS;
      _running = false;
//      _led_count = n;
//      _mode_last_call_time = 0;
//      _mode_delay = 50;
//      _color = DEFAULT_COLOR;
//      _mode_color = DEFAULT_COLOR;
//      _counter_mode_call = 0;
//      _counter_mode_step = 0;
      RESET_RUNTIME;
      _num_segments = 1;
      _segments[0].mode = DEFAULT_MODE;
      _segments[0].color[0] = DEFAULT_COLOR;
      _segments[0].start = 0;
      _segments[0].stop = n - 1;
      _segments[0].speed = DEFAULT_SPEED;
    }

    void
      init(void),
      service(void),
      start(void),
      stop(void),
      setMode(uint8_t m),
      setSpeed(uint16_t s),
      increaseSpeed(uint8_t s),
      decreaseSpeed(uint8_t s),
      setColor(uint8_t r, uint8_t g, uint8_t b),
      setColor(uint32_t c),
      setBrightness(uint8_t b),
      increaseBrightness(uint8_t s),
      decreaseBrightness(uint8_t s),
      setLength(uint16_t b),
      increaseLength(uint16_t s),
      decreaseLength(uint16_t s),
      trigger(void),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,   uint16_t speed, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t colors[], uint8_t num_colors, uint16_t speed, bool reverse);

    boolean
      isRunning(void);

    uint8_t
      getMode(void),
      getBrightness(void),
      getModeCount(void);

    uint16_t
      getSpeed(void),
      getLength(void);

    uint32_t
      color_wheel(uint8_t),
      getColor(void);

    const __FlashStringHelper*
      getModeName(uint8_t m);

  private:
    void
      strip_off(void);

    uint16_t
      mode_static(void),
      mode_blink(void),
      color_wipe(uint32_t, uint32_t),
      mode_color_wipe(void),
      mode_color_wipe_inverse(void),
      mode_color_wipe_random(void),
      mode_random_color(void),
      mode_single_dynamic(void),
      mode_multi_dynamic(void),
      mode_breath(void),
      mode_fade(void),
      mode_scan(void),
      mode_dual_scan(void),
      mode_theater_chase(void),
      mode_theater_chase_rainbow(void),
      mode_rainbow(void),
      mode_rainbow_cycle(void),
      mode_running_lights(void),
      twinkle(uint32_t),
      mode_twinkle(void),
      mode_twinkle_random(void),
      twinkle_fade(uint32_t),
      mode_twinkle_fade(void),
      mode_twinkle_fade_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_multi_strobe(void),
      mode_blink_rainbow(void),
      chase(uint32_t, uint32_t, uint32_t),
      mode_chase_white(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_chase_blackout(void),
      mode_chase_blackout_rainbow(void),
      mode_color_sweep_random(void),
      running(uint32_t, uint32_t),
      mode_running_color(void),
      mode_running_red_blue(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      fireworks(uint32_t),
      mode_fireworks(void),
      mode_fireworks_random(void),
      mode_merry_christmas(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_fire_flicker_soft(void),
      mode_fire_flicker_intense(void),
      fire_flicker(int),
      mode_dual_color_wipe_in_out(void),
      mode_dual_color_wipe_in_in(void),
      mode_dual_color_wipe_out_out(void),
      mode_dual_color_wipe_out_in(void),
      mode_circus_combustus(void),
      tricolor_chase(uint32_t, uint32_t, uint32_t),
      mode_bicolor_chase(void),
      mode_tricolor_chase(void);


    boolean
      _running,
      _triggered;

    uint8_t
      get_random_wheel_index(uint8_t),
//      _mode_index,
//      _speed,
      _brightness;

//    uint16_t
//      _led_count;

//    uint32_t
//      _color,
//      _counter_mode_call,
//      _counter_mode_step,
//      _mode_color,
//      _mode_delay;

//    unsigned long
//      _mode_last_call_time;

    const __FlashStringHelper*
      _name[MODE_COUNT];

    mode_ptr
      _mode[MODE_COUNT];

    uint8_t _segment_index = 0;
    uint8_t _num_segments = 1;
    segment _segments[MAX_NUM_SEGMENTS] = { // must explicitly set array size
      // mode, color[], speed, start, stop, reverse
      { FX_MODE_STATIC, {DEFAULT_COLOR}, DEFAULT_SPEED, 0, 9, false}
    };
    segment_runtime _segment_runtimes[MAX_NUM_SEGMENTS];
};

#endif
