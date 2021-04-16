/*
  WS2812FX example sketch to demo sound reactivity
  
  Keith Lord - 2021
  
  FEATURES
    * use an audio sensor (like those commonly made with an LM393 chip) to add
      sound reactivity to your LED lights. Connect the audio output of the sound
      detector (AO) to your microprocessor's analog input.

    * https://electropeak.com/learn/interfacing-ky-037-sound-sensor-with-arduino/


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2021  Keith Lord 

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
  2021-04-02 initial version
  
*/

#include <WS2812FX.h>

#define LED_PIN          4  // digital pin used to drive the LED strip
#define LED_COUNT      144  // number of LEDs on the strip

#define MIN_BRIGHTNESS   1
#define MAX_BRIGHTNESS 255
#define THRESHOLD        5

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

uint16_t quietLevel = 0;
uint16_t maxSample = 0;
uint16_t maxSampleEver = 0;
unsigned long timer = 0;

void setup() {
  Serial.begin(115200);

  // take some initial audio measurements to establish the "quiet" level
  for(int i=0; i<20; i++) {
    quietLevel += analogRead(A0); // 0-1023
    delay(25);
  }
  quietLevel /= 20;
  Serial.print("\nquietLevel is "); Serial.println(quietLevel);
  
  ws2812fx.init();
  ws2812fx.setBrightness(64);

  // parameters: index, start, stop, mode, color, speed, reverse
  ws2812fx.setSegment(0,  0,  LED_COUNT-1, FX_MODE_STATIC, RED, 8000, NO_OPTIONS);

  ws2812fx.start();
}

void loop() {
  // take an audio sample
  uint16_t audioSample = abs(analogRead(A0) - quietLevel);
  if(audioSample > maxSample) maxSample = audioSample;

  // if the timer has expired, use the sampled audio to recalculate the LED brightness
  if(millis() > timer) {
    if(maxSample > THRESHOLD) { // ensure the audio is above the threshold to reduce LED flickering
      if(maxSample > maxSampleEver) maxSampleEver = maxSample;

      // calculate a new brightness, properly scaled to the sampled audio
      uint8_t newBrightness = map(maxSample, THRESHOLD, maxSampleEver, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
//    Serial.print("maxSample:");Serial.print(maxSample); // debug
//    Serial.print(", maxSampleEver:");Serial.print(maxSampleEver);
//    Serial.print(", newBrightness:");Serial.println(newBrightness);
      ws2812fx.setBrightness(newBrightness);
    } else {
      ws2812fx.setBrightness(MIN_BRIGHTNESS);
    }

    maxSample = 0;
    timer = millis() + 100; // recalc brightness every 100ms
  }

  ws2812fx.service();
}
