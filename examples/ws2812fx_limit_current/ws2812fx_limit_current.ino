/*
  Copy of the serial control example sketch which demonstrates how to use the
  intensitySum() function, along with a custom show() function, to dynamically
  adjust brightness to limit the LEDs' current draw below a set maximum.
  Note, the QUIESCENT_CURRENT and INCREMENTAL_CURRENT #defines were determined
  empirically by taking current measurements with a specific hardware setup. You
  may need to adjust those parameters to reflect your hardware's characteristics.

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
  2018-11-10 initial version
*/

#include <WS2812FX.h>

#define LED_COUNT 30
#define LED_PIN 5
#define MAX_NUM_CHARS 16 // maximum number of characters read from the serial comm

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

char cmd[MAX_NUM_CHARS];    // char[] to store incoming serial commands
bool cmd_complete = false;  // whether the command string is complete

void setup() {
  Serial.begin(115200);
  delay(200); // pause for serial comm to initialize

  ws2812fx.init();
  ws2812fx.setBrightness(64);

  const uint32_t colors[] = { 0x400000, 0x004000, 0x000040 };
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_STATIC, colors, 1000, NO_OPTIONS);

  // set the custom show function
  ws2812fx.setCustomShow(myCustomShow);

  ws2812fx.start();

  printModes();
  printUsage();
}

void loop() {
  ws2812fx.service();

  recvChar(); // read serial comm

  if(cmd_complete) {
    process_command();
  }
}

/*
 * Checks received command and calls corresponding functions.
 */
void process_command() {
  if (strncmp(cmd,"b ",2) == 0) {
    uint8_t b = (uint8_t)atoi(cmd + 2);
    ws2812fx.setBrightness(b);
    Serial.print(F("Set brightness to ")); Serial.println(ws2812fx.getBrightness());
  }

  if (strncmp(cmd,"m ",2) == 0) {
    uint8_t m = (uint8_t)atoi(cmd + 2);
    ws2812fx.setMode(m);
    ws2812fx.resetSegmentRuntimes();

    Serial.print(F("Set mode to ")); Serial.print(ws2812fx.getMode());
    Serial.print(" - "); Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
  }

  if (strncmp(cmd,"s ",2) == 0) {
    uint16_t s = (uint16_t)atoi(cmd + 2);
    ws2812fx.setSpeed(s); 
    Serial.print(F("Set speed to: ")); Serial.println(ws2812fx.getSpeed());
  }

  if (strncmp(cmd,"c ",2) == 0) {
    uint32_t c = (uint32_t)strtoul(cmd + 2, NULL, 16);
    ws2812fx.setColor(c);
    Serial.print(F("Set color to 0x")); Serial.println(ws2812fx.getColor(), HEX);
  }

  cmd[0] = '\0';         // reset the commandstring
  cmd_complete = false;  // reset command complete
}

/*
 * Prints a usage menu.
 */
const char usageText[] PROGMEM = R"=====(
Usage:
m <n> : select mode <n>

b <n> : set brightness to <n>

s <n> : set speed to <n>

c 0x007BFF : set color to 0x007BFF

Have a nice day.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
)=====";

void printUsage() {
  Serial.println((const __FlashStringHelper *)usageText);
}


/*
 * Prints all available WS2812FX modes/effects
 */
void printModes() {
  Serial.println(F("Supporting the following modes: \r\n"));
  for(int i=0; i < ws2812fx.getModeCount(); i++) {
    Serial.print(i); Serial.print('\t'); Serial.println(ws2812fx.getModeName(i));
  }
}


/*
 * Reads new input from serial to cmd string. Command is completed on \n
 */
void recvChar(void) {
  static byte index = 0;
  while (Serial.available() > 0 && cmd_complete == false) {
    char rc = Serial.read();
    if (rc != '\n') {
      if(index < MAX_NUM_CHARS) cmd[index++] = rc;
    } else {
      cmd[index] = '\0'; // terminate the string
      index = 0;
      cmd_complete = true;
      Serial.print("received '"); Serial.print(cmd); Serial.println("'");
    }
  }
}

#define MAX_CURRENT         500 // maximum allowed current draw for the entire strip (mA)
#define QUIESCENT_CURRENT    56 // current draw for the entire strip with all LEDs off (mA)
#define INCREMENTAL_CURRENT  40 // increase in current for each intensity step per RGB color (uA)
#define MAX_INTENSITY_SUM   ((MAX_CURRENT - QUIESCENT_CURRENT) * (uint32_t)1000) / INCREMENTAL_CURRENT
void myCustomShow(void) {
  static uint32_t lastSum = 0;
  uint32_t intensitySum = ws2812fx.intensitySum(); // get intensity sum for all LEDs

  if(lastSum != intensitySum) { // if intesity sum has changed, check if it exceeds the maximum
    if(intensitySum > MAX_INTENSITY_SUM) { // exceeded the maximum, so calculate a new brightness setting
      Serial.print("intensitySum="); Serial.print(intensitySum);
      Serial.print(", MAX_INTENSITY_SUM="); Serial.println(MAX_INTENSITY_SUM);

      uint32_t estimatedCurrent = ((intensitySum * INCREMENTAL_CURRENT) / 1000) + QUIESCENT_CURRENT;
      Serial.print("estimatedCurrent="); Serial.print(estimatedCurrent); Serial.println("mA");

      uint8_t oldBrightness = ws2812fx.Adafruit_NeoPixel::getBrightness();
      Serial.print("old brightness="); Serial.println(oldBrightness);

      uint8_t scaleFactor = (MAX_INTENSITY_SUM * 256) / intensitySum; // brightness scaling factor
      uint8_t newBrightness = constrain(((oldBrightness * scaleFactor) / 256), BRIGHTNESS_MIN, BRIGHTNESS_MAX);
      Serial.print("new brightness="); Serial.println(newBrightness);

      // call the Adafruit_Neopixel::setBrightness() function, because the WS2812FX::setBrightness() function calls show(), which we don't want.
      ws2812fx.Adafruit_NeoPixel::setBrightness(newBrightness);

      // optionally, for verification, recalculate the intensity sum and current estimate
      intensitySum = ws2812fx.intensitySum();
      Serial.print("new intensitySum="); Serial.println(intensitySum);
      estimatedCurrent = ((intensitySum * INCREMENTAL_CURRENT) / 1000) + QUIESCENT_CURRENT;
      Serial.print("new estimatedCurrent="); Serial.print(estimatedCurrent); Serial.println("mA");
    }
    lastSum = intensitySum;
  }
  ws2812fx.Adafruit_NeoPixel::show();
}
