![WS2812FX library](https://raw.githubusercontent.com/kitesurfer1404/WS2812FX/master/WS2812FX_logo.png)

WS2812FX - More Blinken for your LEDs!
======================================

This library features a variety of blinken effects for the WS2811/WS2812/NeoPixel LEDs. It is meant to be a drop-in replacement for the Adafruit NeoPixel library with additional features.

Features
--------

* 53 different effects. And counting.
* Free of any delay()
* Tested on Arduino Nano, Uno, Micro and ESP8266.
* All effects with printable names - easy to use in user interfaces.
* FX, speed and brightness controllable on the fly.
* Ready for sound-to-light (see external trigger example)


Download, Install and Example
-----------------------------

* Install the famous [Adafruit NeoPixel library](https://github.com/adafruit/Adafruit_NeoPixel)
* Download this repository.
* Extract to your Arduino libraries directory.
* Open Arduino IDE.
* Now you can choose File > Examples > WS2812FX > ...

See examples for basic usage.

In it's most simple form, here's the code to get you started!

```cpp
#include <WS2812FX.h>

#define LED_COUNT 30
#define LED_PIN 12

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(200);
  ws2812fx.setMode(FX_MODE_RAINBOW_CYCLE);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
}
```

More complex effects can be created by dividing your string of LEDs into segments (up to ten) and programming each segment independently. Use the **setSegment()** function to program each segment's mode, color, speed and direction (normal or reverse):
  * setSegment(segment index, start LED, stop LED, mode, color, speed, reverse);

Note, some effects make use of more then one color (up to three) and are programmed by specifying an array of colors:
  * setSegment(segment index, start LED, stop LED, mode, colors[], speed, reverse);

```cpp
// divide the string of LEDs into two independent segments
uint32_t colors[] = {RED, GREEN};
ws2812fx.setSegment(0, 0,           (LED_COUNT/2)-1, FX_MODE_BLINK, colors, 1000, false);
ws2812fx.setSegment(1, LED_COUNT/2, LED_COUNT-1,     FX_MODE_BLINK, (const uint32_t[]) {ORANGE, PURPLE}, 1000, false);
```


Effects
-------

* **Static** - No blinking. Just plain old static light.
* **Blink** - Normal blinking. 50% on/off time.
* **Breath** - Does the "standby-breathing" of well known i-Devices. Fixed Speed.
* **Color Wipe** - Lights all LEDs after each other up. Then turns them in that order off. Repeat.
* **Color Wipe Inverse** - Same as Color Wipe, except swaps on/off colors.
* **Color Wipe Reverse** - Lights all LEDs after each other up. Then turns them in reverse order off. Repeat.
* **Color Wipe Reverse Inverse** - Same as Color Wipe Reverse, except swaps on/off colors.
* **Color Wipe Random** - Turns all LEDs after each other to a random color. Then starts over with another color.
* **Random Color** - Lights all LEDs in one random color up. Then switches them to the next random color.
* **Single Dynamic** - Lights every LED in a random color. Changes one random LED after the other to another random color.
* **Multi Dynamic** - Lights every LED in a random color. Changes all LED at the same time to new random colors.
* **Rainbow** - Cycles all LEDs at once through a rainbow.
* **Rainbow Cycle** - Cycles a rainbow over the entire string of LEDs.
* **Scan** - Runs a single pixel back and forth.
* **Dual Scan** - Runs two pixel back and forth in opposite directions.
* **Fade** - Fades the LEDs on and (almost) off again.
* **Theater Chase** - Theatre-style crawling lights. Inspired by the Adafruit examples.
* **Theater Chase Rainbow** - Theatre-style crawling lights with rainbow effect. Inspired by the Adafruit examples.
* **Running Lights** - Running lights effect with smooth sine transition.
* **Twinkle** - Blink several LEDs on, reset, repeat.
* **Twinkle Random** - Blink several LEDs in random colors on, reset, repeat.
* **Twinkle Fade** - Blink several LEDs on, fading out.
* **Twinkle Fade Random** - Blink several LEDs in random colors on, fading out.
* **Sparkle** - Blinks one LED at a time.
* **Flash Sparkle** - Lights all LEDs in the selected color. Flashes single white pixels randomly.
* **Hyper Sparkle** - Like flash sparkle. With more flash.
* **Strobe** - Classic Strobe effect.
* **Strobe Rainbow** - Classic Strobe effect. Cycling through the rainbow.
* **Multi Strobe** - Strobe effect with different strobe count and pause, controlled by speed setting.
* **Blink Rainbow** - Classic Blink effect. Cycling through the rainbow.
* **Chase White** - Color running on white.
* **Chase Color** - White running on color.
* **Chase Random** - White running followed by random color.
* **Chase Rainbow** - White running on rainbow.
* **Chase Flash** - White flashes running on color.
* **Chase Flash Random** - White flashes running, followed by random color.
* **Chase Rainbow White** - Rainbow running on white.
* **Chase Blackout** - Black running on color.
* **Chase Blackout Rainbow** - Black running on rainbow.
* **Color Sweep Random** - Random color introduced alternating from start and end of strip.
* **Running Color** - Alternating color/white pixels running.
* **Running Red Blue** - Alternating red/blue pixels running.
* **Running Random** - Random colored pixels running.
* **Larson Scanner** - K.I.T.T.
* **Comet** - Firing comets from one end.
* **Fireworks** - Firework sparks.
* **Fireworks Random** - Random colored firework sparks.
* **Merry Christmas** - Alternating green/red pixels running.
* **Fire Flicker** - Fire flickering effect. Like in harsh wind.
* **Fire Flicker (soft)** - Fire flickering effect. Runs slower/softer.
* **Fire Flicker (intense)** - Fire flickering effect. More range of color.
* **Circus Combustus** - Alternating white/red/black pixels running.
* **Halloween** - Alternating orange/purple pixels running.
* **Bicolor Chase** - Two LEDs running on a background color (set three colors).
* **Tricolor Chase** - Alternating three color pixels running (set three colors).
* **ICU** - Two eyes looking around.

Projects using WS2812FX
-----------------------

* [Smart Home project by renat2985](https://github.com/renat2985/rgb) using the ESP8266. Including a nice webinterface in the demo video!
* [WiFi LED Star by kitesurfer1404](http://www.kitesurfer1404.de/tech/led-star/en)
* [McLighting by toblum](https://github.com/toblum/McLighting) is a multi-client lighting gadget for ESP8266
