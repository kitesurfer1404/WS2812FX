/*
  WS2812FX lights with sound effects demo.
  
  Keith Lord - 2018
  
  FEATURES
    * example of running an LED animation with sound effects.
    * The WAV audio files are stored in the ESP8266's LittleFS 
    * filesystem and need to be uploaded to the ESP before
    * uploading the sketch, as described here:
    * https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system
    * 
    * We're using the fantastic ESP8266Audio library available here:
    * https://github.com/earlephilhower/ESP8266Audio.
    * This library uses the ESP8266's I2S digital audio 
    * feature to play audio, which requires a hardware I2S 
    * decoder (like the inexpensive PCM5102A) or the simple 
    * single-transistor output filter described in the ESP8266Audio 
    * documentation. If you don't have a I2S DAC or a transistor 
    * laying around, you can use the two-capacitor-one-resistor 
    * output filter that's described here:
    * http://blog.dspsynth.eu/audio-hacking-on-the-esp8266/
    * 
    * Note, the I2S output (RX pin) is shared with the Serial
    * port, so if you're not using a hardware DAC and you've got 
    * the RX pin driving a speaker, don't be surprised to hear 
    * garbled audio when the Serial port is in use (i.e. when a 
    * sketch is uploaded to the ESP8266).
    * 
    * Also note, as you can imagine playing audio is a time 
    * sensitive task as is updating ws2812 LEDs. Sometimes the two
    * will clash, for eample if you use a very long strip of LEDs,
    * the I2S buffer will be starved for data during the time the 
    * LED strip is being updated. Depending on what you're using as 
    * a DAC, this will cause clicks and pops in the audio, or your
    * audio will start sounding like a Dalek. In my testing 100 
    * LEDs works fine, but 200 LEDs causes clicks and pops.

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2018  Keith Lord 

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
  2018-8-30 initial version
  
*/

#include <WS2812FX.h>
#include "AudioFileSourceLittleFS.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

#define LED_PIN    5 // digital pin used to drive the LED strip
#define LED_COUNT 30 // number of LEDs on the strip

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

AudioGeneratorWAV *wav = NULL;
AudioFileSourceLittleFS *file = NULL;
AudioOutputI2S *out = NULL;

const char* soundfx = "/pew.wav";

void setup() {
  Serial.begin(115200);

  LittleFS.begin(); // init LittleFS file system where the audio samples are stored

  ws2812fx.init();
  ws2812fx.setBrightness(255);

  // parameters: index, start, stop, mode, color, speed, options
  uint32_t colors[] = { RED, GREEN, BLUE };
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_LARSON_SCANNER, colors, 3000, NO_OPTIONS);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();

  if(ws2812fx.isFrame() && ws2812fx.isCycle()) {
    playSound();
  }

  if (wav != NULL && wav->isRunning()) { // feed the I2S buffer
    if (!wav->loop()) wav->stop();
  }
}

void playSound() {
  if(wav != NULL && wav->isRunning()) wav->stop();

  if(file != NULL) delete file;
  if(out  != NULL) delete out;
  if(wav  != NULL) delete wav;

  file = new AudioFileSourceLittleFS(soundfx);
  out = new AudioOutputI2S();
  wav = new AudioGeneratorWAV();

  wav->begin(file, out);
}
