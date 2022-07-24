/*
  WS2812FX.h - Library for WS2812 LED effects.

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
*/

#ifndef WS2812FX_h
#define WS2812FX_h

#define FSH(x) (__FlashStringHelper*)(x)
#define MAX_MILLIS (0UL - 1UL) /* ULONG_MAX */

#include <Adafruit_NeoPixel.h>

#define DEFAULT_BRIGHTNESS (uint8_t)50
#define DEFAULT_MODE       (uint8_t)0
#define DEFAULT_SPEED      (uint16_t)1000
#define DEFAULT_COLOR      (uint32_t)0xFF0000
#define DEFAULT_COLORS     { RED, GREEN, BLUE }
#define COLORS(...)        (const uint32_t[]){__VA_ARGS__}

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  //#pragma message("Compiling for ESP or RP2040")
  #define SPEED_MIN (uint16_t)2
#else
  //#pragma message("Compiling for Arduino")
  #define SPEED_MIN (uint16_t)10
#endif
#define SPEED_MAX (uint16_t)65535

#define BRIGHTNESS_MIN (uint8_t)0
#define BRIGHTNESS_MAX (uint8_t)255

/* each segment uses 36 bytes of SRAM memory, so if you're compile fails
  because of insufficient flash memory, decreasing MAX_NUM_SEGMENTS may help */
#define MAX_NUM_SEGMENTS         10
#define MAX_NUM_ACTIVE_SEGMENTS  10
#define INACTIVE_SEGMENT        255 /* max uint_8 */
#define MAX_NUM_COLORS            3 /* number of colors per segment */
#define MAX_CUSTOM_MODES          8

// some common colors
#define RED        (uint32_t)0xFF0000
#define GREEN      (uint32_t)0x00FF00
#define BLUE       (uint32_t)0x0000FF
#define WHITE      (uint32_t)0xFFFFFF
#define BLACK      (uint32_t)0x000000
#define YELLOW     (uint32_t)0xFFFF00
#define CYAN       (uint32_t)0x00FFFF
#define MAGENTA    (uint32_t)0xFF00FF
#define PURPLE     (uint32_t)0x400080
#define ORANGE     (uint32_t)0xFF3000
#define PINK       (uint32_t)0xFF1493
#define GRAY       (uint32_t)0x101010
#define ULTRAWHITE (uint32_t)0xFFFFFFFF
#define DIM(c)     (uint32_t)((c >> 2) & 0x3f3f3f3f) // color at 25% intensity
#define DARK(c)    (uint32_t)((c >> 4) & 0x0f0f0f0f) // color at  6% intensity

// segment options
// bit    7: reverse animation
// bits 4-6: fade rate (0-7)
// bit    3: gamma correction
// bits 1-2: size
// bits   0: TBD
#define NO_OPTIONS   (uint8_t)0b00000000
#define REVERSE      (uint8_t)0b10000000
#define IS_REVERSE   ((_seg->options & REVERSE) == REVERSE)
#define FADE_XFAST   (uint8_t)0b00010000
#define FADE_FAST    (uint8_t)0b00100000
#define FADE_MEDIUM  (uint8_t)0b00110000
#define FADE_SLOW    (uint8_t)0b01000000
#define FADE_XSLOW   (uint8_t)0b01010000
#define FADE_XXSLOW  (uint8_t)0b01100000
#define FADE_GLACIAL (uint8_t)0b01110000
#define FADE_RATE    ((_seg->options >> 4) & 7)
#define GAMMA        (uint8_t)0b00001000
#define IS_GAMMA     ((_seg->options & GAMMA) == GAMMA)
#define SIZE_SMALL   (uint8_t)0b00000000
#define SIZE_MEDIUM  (uint8_t)0b00000010
#define SIZE_LARGE   (uint8_t)0b00000100
#define SIZE_XLARGE  (uint8_t)0b00000110
#define SIZE_OPTION  ((_seg->options >> 1) & 3)

// segment runtime options (aux_param2)
#define FRAME           (uint8_t)0b10000000
#define SET_FRAME       (_seg_rt->aux_param2 |=  FRAME)
#define CLR_FRAME       (_seg_rt->aux_param2 &= ~FRAME)
#define CYCLE           (uint8_t)0b01000000
#define SET_CYCLE       (_seg_rt->aux_param2 |=  CYCLE)
#define CLR_CYCLE       (_seg_rt->aux_param2 &= ~CYCLE)
#define CLR_FRAME_CYCLE (_seg_rt->aux_param2 &= ~(FRAME | CYCLE))

class WS2812FX : public Adafruit_NeoPixel {

  public:
    typedef uint16_t (WS2812FX::*mode_ptr)(void);

    // segment parameters
    typedef struct Segment { // 20 bytes
      uint16_t start;
      uint16_t stop;
      uint16_t speed;
      uint8_t  mode;
      uint8_t  options;
      uint32_t colors[MAX_NUM_COLORS];
    } segment;

    // segment runtime parameters
    typedef struct Segment_runtime { // 20 bytes for Arduino, 24 bytes for ESP
      unsigned long next_time;
      uint32_t counter_mode_step;
      uint32_t counter_mode_call;
      uint8_t  aux_param;   // auxilary param (usually stores a color_wheel index)
      uint8_t  aux_param2;  // auxilary param (usually stores bitwise options)
      uint16_t aux_param3;  // auxilary param (usually stores a segment index)
      uint8_t* extDataSrc = NULL; // external data array
      uint16_t extDataCnt = 0;    // number of elements in the external data array
    } segment_runtime;

    WS2812FX(uint16_t num_leds, uint8_t pin, neoPixelType type,
      uint8_t max_num_segments=MAX_NUM_SEGMENTS,
      uint8_t max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS)
      : Adafruit_NeoPixel(num_leds, pin, type) {

      brightness = DEFAULT_BRIGHTNESS + 1; // Adafruit_NeoPixel internally offsets brightness by 1
      _running = false;

      _segments_len = max_num_segments;
      _active_segments_len = max_num_active_segments;

      // create all the segment arrays and init to zeros
      _segments = new segment[_segments_len]();
      _active_segments = new uint8_t[_active_segments_len]();
      _segment_runtimes = new segment_runtime[_active_segments_len]();

      // init segment pointers
      _seg     = _segments;
      _seg_rt  = _segment_runtimes;

      resetSegments();
      setSegment(0, 0, num_leds - 1, DEFAULT_MODE, DEFAULT_COLOR, DEFAULT_SPEED, NO_OPTIONS);
    };

    void
//    timer(void),
      init(void),
      start(void),
      stop(void),
      pause(void),
      resume(void),
      strip_off(void),
      fade_out(void),
      fade_out(uint32_t),
      setMode(uint8_t m),
      setMode(uint8_t seg, uint8_t m),
      setOptions(uint8_t seg, uint8_t o),
      setCustomMode(uint16_t (*p)()),
      setCustomShow(void (*p)()),
      setSpeed(uint16_t s),
      setSpeed(uint8_t seg, uint16_t s),
      increaseSpeed(uint8_t s),
      decreaseSpeed(uint8_t s),
      setColor(uint8_t r, uint8_t g, uint8_t b),
      setColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      setColor(uint32_t c),
      setColor(uint8_t seg, uint32_t c),
      setColors(uint8_t seg, uint32_t* c),
      setBrightness(uint8_t b),
      increaseBrightness(uint8_t s),
      decreaseBrightness(uint8_t s),
      setLength(uint16_t b),
      increaseLength(uint16_t s),
      decreaseLength(uint16_t s),
      trigger(void),
      setCycle(void),
      setNumSegments(uint8_t n),

      setSegment(),
      setSegment(uint8_t n),
      setSegment(uint8_t n, uint16_t start),
      setSegment(uint8_t n, uint16_t start, uint16_t stop),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color, uint16_t speed, uint8_t options),

      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[]),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, bool reverse),
      setSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),

      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed),
      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, uint32_t color,          uint16_t speed, uint8_t options),
      setIdleSegment(uint8_t n, uint16_t start, uint16_t stop, uint8_t mode, const uint32_t colors[], uint16_t speed, uint8_t options),
      addActiveSegment(uint8_t seg),
      removeActiveSegment(uint8_t seg),
      swapActiveSegment(uint8_t oldSeg, uint8_t newSeg),

      resetSegments(void),
      resetSegmentRuntimes(void),
      resetSegmentRuntime(uint8_t),
      setPixelColor(uint16_t n, uint32_t c),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b),
      setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w),
      setRawPixelColor(uint16_t n, uint32_t c),
      copyPixels(uint16_t d, uint16_t s, uint16_t c),
      setPixels(uint16_t, uint8_t*),
      setRandomSeed(uint16_t),
      setExtDataSrc(uint8_t seg, uint8_t *src, uint8_t cnt),
      show(void);

    bool
      service(void),
      isRunning(void),
      isTriggered(void),
      isFrame(void),
      isFrame(uint8_t),
      isCycle(void),
      isCycle(uint8_t),
      isActiveSegment(uint8_t seg);

    uint8_t
      random8(void),
      random8(uint8_t),
      getMode(void),
      getMode(uint8_t),
      getModeCount(void),
      setCustomMode(const __FlashStringHelper* name, uint16_t (*p)()),
      setCustomMode(uint8_t i, const __FlashStringHelper* name, uint16_t (*p)()),
      getNumSegments(void),
      get_random_wheel_index(uint8_t),
      getOptions(uint8_t),
      getNumBytesPerPixel(void);

    uint16_t
      random16(void),
      random16(uint16_t),
      getSpeed(void),
      getSpeed(uint8_t),
      getLength(void),
      getNumBytes(void);

    uint32_t
      color_wheel(uint8_t),
      getColor(void),
      getColor(uint8_t),
      intensitySum(void);

    uint32_t* getColors(uint8_t);
    uint32_t* intensitySums(void);
    uint8_t*  getActiveSegments(void);
    uint8_t*  blend(uint8_t*, uint8_t*, uint8_t*, uint16_t, uint8_t);

    const __FlashStringHelper* getModeName(uint8_t m);

    WS2812FX::Segment* getSegment(void);

    WS2812FX::Segment* getSegment(uint8_t);

    WS2812FX::Segment* getSegments(void);

    WS2812FX::Segment_runtime* getSegmentRuntime(void);

    WS2812FX::Segment_runtime* getSegmentRuntime(uint8_t);

    WS2812FX::Segment_runtime* getSegmentRuntimes(void);

    // mode helper functions
    uint16_t
      blink(uint32_t, uint32_t, bool strobe),
      color_wipe(uint32_t, uint32_t, bool),
      twinkle(uint32_t, uint32_t),
      twinkle_fade(uint32_t),
      sparkle(uint32_t, uint32_t),
      chase(uint32_t, uint32_t, uint32_t),
      chase_flash(uint32_t, uint32_t),
      running(uint32_t, uint32_t),
      fireworks(uint32_t),
      fire_flicker(int),
      tricolor_chase(uint32_t, uint32_t, uint32_t),
      scan(uint32_t, uint32_t, bool);

    uint32_t
      color_blend(uint32_t, uint32_t, uint8_t),
      getRawPixelColor(uint16_t n);

    // builtin modes
    uint16_t
      mode_static(void),
      mode_blink(void),
      mode_blink_rainbow(void),
      mode_strobe(void),
      mode_strobe_rainbow(void),
      mode_color_wipe(void),
      mode_color_wipe_inv(void),
      mode_color_wipe_rev(void),
      mode_color_wipe_rev_inv(void),
      mode_color_wipe_random(void),
      mode_color_sweep_random(void),
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
      mode_twinkle(void),
      mode_twinkle_random(void),
      mode_twinkle_fade(void),
      mode_twinkle_fade_random(void),
      mode_sparkle(void),
      mode_flash_sparkle(void),
      mode_hyper_sparkle(void),
      mode_multi_strobe(void),
      mode_chase_white(void),
      mode_chase_color(void),
      mode_chase_random(void),
      mode_chase_rainbow(void),
      mode_chase_flash(void),
      mode_chase_flash_random(void),
      mode_chase_rainbow_white(void),
      mode_chase_blackout(void),
      mode_chase_blackout_rainbow(void),
      mode_running_color(void),
      mode_running_red_blue(void),
      mode_running_random(void),
      mode_larson_scanner(void),
      mode_comet(void),
      mode_fireworks(void),
      mode_fireworks_random(void),
      mode_merry_christmas(void),
      mode_halloween(void),
      mode_fire_flicker(void),
      mode_fire_flicker_soft(void),
      mode_fire_flicker_intense(void),
      mode_circus_combustus(void),
      mode_bicolor_chase(void),
      mode_tricolor_chase(void),
      mode_twinkleFOX(void),
      mode_rain(void),
      mode_block_dissolve(void),
      mode_icu(void),
      mode_dual_larson(void),
      mode_running_random2(void),
      mode_filler_up(void),
      mode_rainbow_larson(void),
      mode_rainbow_fireworks(void),
      mode_trifade(void),
      mode_vu_meter(void),
      mode_heartbeat(void),
      mode_bits(void),
      mode_multi_comet(void),
      mode_flipbook(void),
      mode_popcorn(void),
      mode_oscillator(void),
      mode_custom_0(void),
      mode_custom_1(void),
      mode_custom_2(void),
      mode_custom_3(void),
      mode_custom_4(void),
      mode_custom_5(void),
      mode_custom_6(void),
      mode_custom_7(void);

  private:
    uint16_t _rand16seed;
    uint16_t (*customModes[MAX_CUSTOM_MODES])(void) {
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; },
      []{ return (uint16_t)1000; }
    };
    void (*customShow)(void) = NULL;

    bool
      _running,
      _triggered;

    segment* _segments;                 // array of segments (20 bytes per element)
    segment_runtime* _segment_runtimes; // array of segment runtimes (16 bytes per element)
    uint8_t* _active_segments;          // array of active segments (1 bytes per element)

    uint8_t _segments_len = 0;          // size of _segments array
    uint8_t _active_segments_len = 0;   // size of _segments_runtime and _active_segments arrays
    uint8_t _num_segments = 0;          // number of configured segments in the _segments array

    segment* _seg;                      // currently active segment (20 bytes)
    segment_runtime* _seg_rt;           // currently active segment runtime (16 bytes)

    uint16_t _seg_len;                  // num LEDs in the currently active segment
};

class WS2812FXT {
  public:
    WS2812FXT(uint16_t num_leds, uint8_t pin, neoPixelType type,
      uint8_t max_num_segments=MAX_NUM_SEGMENTS,
      uint8_t max_num_active_segments=MAX_NUM_ACTIVE_SEGMENTS) {
        v1 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
        v2 = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments);
        dest = new WS2812FX(num_leds, pin, type, max_num_segments, max_num_active_segments); 
    };

    void init(void) {
      v1->init();
      v2->init();
      v1->setCustomShow([]{ return; });
      v2->setCustomShow([]{ return; });
    }

    void start(void) {
      v1->start();
      v2->start();
    }

    void service(void) {
      bool doShow = v1->service() || v2->service();
      if(doShow) {
        _show();
      }
    }

    void startTransition(uint16_t duration, bool direction = true) {
      transitionStartTime = millis();
      transitionDuration = duration;
      transitionDirection = direction;
    }

  private:
    void _show(void) {
      unsigned long now = millis();

      uint8_t *dest_p = dest->getPixels();
      uint8_t *vstart_p = transitionDirection ? v1->getPixels() : v2->getPixels();
      uint8_t *vstop_p  = transitionDirection ? v2->getPixels() : v1->getPixels();
      uint16_t numBytes = dest->getNumBytes();

      if(now < transitionStartTime) {
        memmove(dest_p, vstart_p, numBytes);
      } else if(now > transitionStartTime + transitionDuration) {
        memmove(dest_p, vstop_p, numBytes);
      } else {
        uint8_t blendAmt = map(now, transitionStartTime, transitionStartTime + transitionDuration, 0, 255);
        dest->blend(dest_p, vstart_p, vstop_p, numBytes, blendAmt);
      }

      dest->Adafruit_NeoPixel::show();
    }

  public:
    WS2812FX* v1 = NULL;
    WS2812FX* v2 = NULL;
    WS2812FX* dest = NULL;
    unsigned long transitionStartTime = MAX_MILLIS;
    uint16_t transitionDuration = 5000;
    bool transitionDirection = true;
};

// data struct used by the flipbook effect
struct Flipbook {
  int8_t   numPages;
  int8_t   numRows;
  int8_t   numCols;
  uint32_t* colors;
};

// data struct used by the popcorn effect
struct Popcorn {
  float position;
  float velocity;
  int32_t color;
};

// data struct used by the oscillator effect
struct Oscillator {
  uint8_t size;
  int16_t pos;
  int8_t  speed;
};

#if defined(ESP8266) || defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
  #include "modes_esp.h"
#else
  #include "modes_arduino.h"
#endif

#endif
