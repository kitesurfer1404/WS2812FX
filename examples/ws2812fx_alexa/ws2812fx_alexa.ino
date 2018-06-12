/*
  This sketch demonstrates how to control your WS2812FX lights with your voice using
  an Amazon Echo and Amazon's digital assistant, Alexa. 

  The sketch uses Aircoookie's excellent Espalexa library, which you can download here:
  https://github.com/Aircoookie/Espalexa. After downloading it, install it using the
  Arduino IDE Library Manager's "Add .ZIP library" feature.

  *** HACK ALERT ***
  There's an issue with the ESP8266WebServer code shipped with recent versions of the
  ESP8266 Arduino Core package (versions >= v2.4.0). The bug affects Alexa's ability to
  discover compatible devices on the network and/or your ESP's ability to respond to
  Alexa's commands. There's two workarounds, pick whichever one suits you:
    1) drop back to using ESP8266 Core v2.3.0, which doesn't seem to have the issue.
    2) patch the ESP8266WebServer Parsing.cpp file as described in this thread:
       https://github.com/Aircoookie/Espalexa/issues/6#issuecomment-366533897

  I opted for workaround #2 and my 2nd Gen Echo Dot (fireware version 611498620) worked
  fine. Note, on my Mac setup the file is located at
  /Users/{userid}/Library/Arduino15/packages/esp8266/hardware/esp8266/2.4.1/libraries/ESP8266WebServer/src/Parsing.cpp
  and the patch needs to be applied to line 198.

  Hopefully this will be fixed in the ESP8266 Core v2.5.0 release or perhaps it
  will be fixed with a future Amazon Echo firmware update.
  *** END HACK ALERT ***

  After uploading the sketch to your ESP, you'll need to ask Alexa to discover the
  device on your network by saying "Alexa, discover devices". After a few seconds,
  hopefully, Alexa will respond that it found the "devices" defined in the sketch.
  If Alexa responds with something about making sure you've enabled the SmartHome
  skill appropriate for your device, ignore it. A skill is not needed for the
  Espalexa library to do its thing.

  The sketch defines a set of virtual devices and WS2812FX presets which will be
  activated when Alexa hears the device's name in a verbal command. After Alexa
  has found your devices on the network, you can say things like
  "Alexa, turn on christmas lights"
  "Alexa, turn on christmas lights 30 percent" (sets the brightness to 30%)
  "Alexa, turn off christmas lights"

  You can also control your lights with the Alexa app on your smartphone. Look for your
  virtual devices in the Smart Home section.

  See the Espalexa documentation for more details.
  
  Keith Lord - 2018

  LICENSE

  The MIT License (MIT)

  Copyright (c) 2018  Keith Lord 

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
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
  2018-06-10 initial version
*/

#ifdef ARDUINO_ARCH_ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <Espalexa.h>
#include <WS2812FX.h>

#define NUMLEDS 30 // number of LEDs on the strip
#define DATAPIN  5 // GPIO pin used to drive the LED strip

#define WIFI_SSID     "xxxxxxxx" // WiFi network
#define WIFI_PASSWORD "xxxxxxxx" // WiFi network password

/* 
 *  define your ws2812fx presets
 */
WS2812FX::segment color_preset[MAX_NUM_SEGMENTS] = {
  // { first, last, speed, mode, options, colors[] }
  {0, NUMLEDS-1, 5000, FX_MODE_STATIC, NO_OPTIONS, {RED, GREEN, BLUE}}
};

WS2812FX::segment christmas_preset[MAX_NUM_SEGMENTS] = {
  // { first, last, speed, mode, options, colors[] }
  {0, NUMLEDS-1, 5000, FX_MODE_MERRY_CHRISTMAS, NO_OPTIONS, {RED, GREEN, BLACK}}
};

WS2812FX::segment team_preset[MAX_NUM_SEGMENTS] = {
  // { first, last, speed, mode, options, colors[] }
  {0, NUMLEDS-1, 5000, FX_MODE_TRICOLOR_CHASE, NO_OPTIONS, {0x805000, 0x805000, 0x000040}} // blue and gold, GO ND!
};

WS2812FX::segment flag_preset[MAX_NUM_SEGMENTS] = {
  // { first, last, speed, mode, options, colors[] }
  {0,             (NUMLEDS/6)*1-1,  500, FX_MODE_FLASH_SPARKLE, NO_OPTIONS, {BLUE,  BLACK, BLACK}},
  {(NUMLEDS/6)*1, (NUMLEDS/6)*2-1,  500, FX_MODE_FLASH_SPARKLE, NO_OPTIONS, {BLUE,  BLACK, BLACK}},
  {(NUMLEDS/6)*2, (NUMLEDS/6)*3-1, 2000, FX_MODE_COLOR_WIPE,    NO_OPTIONS, {RED,   WHITE, BLACK}},
  {(NUMLEDS/6)*3, (NUMLEDS/6)*4-1, 2000, FX_MODE_COLOR_WIPE,    NO_OPTIONS, {WHITE, RED,   BLACK}},
  {(NUMLEDS/6)*4, (NUMLEDS/6)*5-1, 2000, FX_MODE_COLOR_WIPE,    NO_OPTIONS, {RED,   WHITE, BLACK}},
  {(NUMLEDS/6)*5, NUMLEDS-1,       2000, FX_MODE_COLOR_WIPE,    NO_OPTIONS, {WHITE, RED,   BLACK}}
};

/*
 * define the ws2812fx and espalexa objects
 */
Espalexa espalexa;
WS2812FX ws2812fx = WS2812FX(NUMLEDS, DATAPIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.println("\r\n");

  // init WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to " WIFI_SSID);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nServer IP is ");
  Serial.println(WiFi.localIP());

  // add your alexa virtual devices giving them a name and associated callback
  espalexa.addDevice("L E D lights", ledCallback);
  espalexa.addDevice("color lights", colorCallback);
  espalexa.addDevice("flag lights", flagCallback);
  espalexa.addDevice("team lights", teamCallback);
  espalexa.addDevice("christmas lights", christmasCallback);

  espalexa.begin();

  ws2812fx.init();
  ws2812fx.stop(); // ws2812fx is stopped until it receives a command from ALexa 
}
 
void loop() {
   espalexa.loop();
   ws2812fx.service();
   delay(1);
}

/*
 * our callback functions
 */
void ledCallback(uint8_t brightness) { // used for on, off or adjusting brightness without changing the active preset
  Serial.print("Loading led ");Serial.println(brightness);
  loadPreset(NULL, brightness);
}

void colorCallback(uint8_t brightness) {
  Serial.print("Loading color preset ");Serial.println(brightness);
  loadPreset(color_preset, brightness);
}

void christmasCallback(uint8_t brightness) {
  Serial.print("Loading christmas preset ");Serial.println(brightness);
  loadPreset(christmas_preset, brightness);
}

void teamCallback(uint8_t brightness) {
  Serial.print("Loading team colors preset ");Serial.println(brightness);
  loadPreset(team_preset, brightness);
}

void flagCallback(uint8_t brightness) {
  Serial.print("Loading flag preset ");Serial.println(brightness);
  loadPreset(flag_preset, brightness);
}

/*
 * function loads and runs the specified ws2812fx preset
 */
void loadPreset(WS2812FX::segment segments[], uint8_t brightness) {
  ws2812fx.stop();
  if(brightness == 0) return;

  if(segments != NULL) {
    ws2812fx.resetSegments();
    for(int i=0; i<MAX_NUM_SEGMENTS; i++) {
      WS2812FX::segment seg = segments[i];
      if(i != 0 && seg.start == 0) break;
      ws2812fx.setSegment(i, seg.start, seg.stop, seg.mode, seg.colors, seg.speed, seg.options);
    }
  }

  ws2812fx.setBrightness(brightness);
  ws2812fx.start();
}
