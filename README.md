WS2812FX - More Blinken for your LEDs!
======================================

This library features a variety of blinken effects for the WS2811/WS2812/NeoPixel LEDs. It is meant to be a drop-in replacement for the Adafruit NeoPixel library with additional features.

Features
--------

* 25 different effects. And counting.
* Free of any delay()
* Tested on Arduino Nano, Uno and ESP8266.
* All effects with printable names - easy to use in user interfaces.
* FX, speed and brightness controllable on the fly.


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


Effects
-------

* **Static** - No blinking. Just plain old static light.
* **Blink** - Normal blinking. 50% on/off time.
* **Breath** - Does the "standby-breathing" of well known i-Devices. Fixed Speed.
* **Color Wipe** - Lights all LEDs after each other up. Then turns them in that order off. Repeat.
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
* **Sparkle** - Blinks one LED at a time.
* **Flash Sparkle** - Lights all LEDs in the selected color. Flashes single white pixels randomly.
* **Hyper Sparkle** - Like flash sparkle. With more flash.
* **Strobe** - Classic Strobe effect.
* **Rainbow Strobe** - Classic Strobe effect. Cycling through the rainbow.
* **Multi Strobe** - Strobe effect with different strobe count and pause, controled by speed setting.
* **Rainbow Blink** - Classic Blink effect. Cycling through the rainbow.
