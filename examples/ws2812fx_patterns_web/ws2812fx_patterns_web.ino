/*
  This sketch introduces the idea of patterns.
  A pattern is a collection of segments. Each pattern plays for a specific
  duration and you can setup an array of patterns that play in a continuous loop.
  A web API is included which receives and stores pattern data in JSON format.
  It's the companion sketch for the LEDfx Android app available in the Google Play
  app store: https://play.google.com/store/apps/details?id=com.champlainsystems.ledfx

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
  2018-02-21 initial version
  2018-11-30 added custom aux functions and OTA update
*/

#include <WS2812FX.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#define VERSION "2.1.0"

uint8_t  dataPin = D1; // default digital pin used to drive the LED strip
uint16_t numLeds = 30; // default number of LEDs on the strip

#define WIFI_SSID "xxxxxxxx"     // WiFi network
#define WIFI_PASSWORD "xxxxxxxx" // WiFi network password
#define HTTP_PORT 80

#define MAX_NUM_PATTERNS 8
//                       duration, brightness, numSegments, [ { first, last, speed, mode, options, colors[] } ]
#define DEFAULT_PATTERN {30, 64, 1, { {0, (uint16_t)(numLeds-1), (uint16_t)(numLeds*20), FX_MODE_STATIC, NO_OPTIONS, {RED,  BLACK, BLACK}} }}

typedef struct Pattern { // 208 bytes/pattern
  int duration;
  uint8_t brightness;
  uint8_t numSegments;
  WS2812FX::segment segments[MAX_NUM_SEGMENTS];
} pattern;

// setup a default patterns array
Pattern patterns[MAX_NUM_PATTERNS] = { DEFAULT_PATTERN };
int numPatterns = 1;
int currentPattern = 0;
unsigned long lastTime = 0;

WS2812FX ws2812fx = WS2812FX(numLeds, dataPin, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(HTTP_PORT);

void (*customAuxFunc[])(void) { // define custom auxiliary functions here
  [] { Serial.println("running customAuxFunc[0]"); },
  [] { Serial.println("running customAuxFunc[1]"); },
  [] { Serial.println("running customAuxFunc[2]"); }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\r\n");

  EEPROM.begin(2048); // for ESP8266 (comment out if using an Arduino)

  // init WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to " WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nServer IP is ");
  Serial.println(WiFi.localIP());

  // init OTA
  ArduinoOTA.onStart([]() {
    Serial.println("OTA start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA end");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  // init LED strip with a default segment
  ws2812fx.init();
  ws2812fx.setBrightness(128);
  ws2812fx.setSegment(0, 0, numLeds - 1, FX_MODE_STATIC, RED, 3000, false);

  // if not rebooting due to catastrophic error, restore pattern data from eeprom
  struct  rst_info  *rstInfo = system_get_rst_info();
  //Serial.print("rstInfo->reason:"); Serial.println(rstInfo->reason);
  if (rstInfo->reason !=  REASON_EXCEPTION_RST) { // not reason 2
    restoreFromEEPROM();
  }

  ws2812fx.start();

  // config and start the web server
  configServer();
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  ws2812fx.service();
  server.handleClient();

  // if it's time to change pattern, do it now
  unsigned long now = millis();
  if (lastTime == 0 || (now - lastTime > patterns[currentPattern].duration * 1000ul)) {
    ws2812fx.clear();
    ws2812fx.resetSegments();

    currentPattern = (currentPattern + 1) % numPatterns;
    ws2812fx.setBrightness(patterns[currentPattern].brightness);
    for (int i = 0; i < patterns[currentPattern].numSegments; i++) {
      WS2812FX::segment seg = patterns[currentPattern].segments[i];
      ws2812fx.setSegment(i, seg.start, seg.stop, seg.mode, seg.colors, seg.speed, seg.options);
    }
    lastTime = now;
  }
}

void configServer() {
  server.onNotFound([]() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(404, "text/plain", "Page not found");
  });

  // return the WS2812FX status
  // optionally set the running state or run custom auxiliary functions
  server.on("/status", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    String running = server.arg("running");
    if (running.length() > 0) {
      if (running == "true") ws2812fx.start();
      else ws2812fx.stop();
    }

    String auxFunc = server.arg("auxFunc");
    if (auxFunc.length() > 0) {
      int auxFuncIndex = auxFunc.toInt();
      if (auxFuncIndex >= 0 && (size_t)auxFuncIndex < sizeof(customAuxFunc) / sizeof(customAuxFunc[0])) {
        customAuxFunc[auxFuncIndex]();
      }
    }

    char status[50] = "{\"version\":\"";
    strcat(status, VERSION);
    strcat(status, "\",\"isRunning\":");
    strcat(status, ws2812fx.isRunning() ? "true" : "false");
    strcat(status, "}");
    server.send(200, "application/json", status);
  });

  // send the WS2812FX mode info in JSON format
  server.on("/getModes", []() {
    char modes[1000] = "[";
    for (uint8_t i = 0; i < ws2812fx.getModeCount(); i++) {
      strcat(modes, "\"");
      strcat_P(modes, (PGM_P)ws2812fx.getModeName(i));
      strcat(modes, "\",");
    }
    modes[strlen(modes) - 1] = ']';

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", modes);
  });

  server.on("/upload", HTTP_OPTIONS, []() { // CORS preflight request
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    server.send(200, "text/plain", "OK");
  });

  // receive the device info in JSON format and update the pattern data
  server.on("/upload", HTTP_POST, []() {
    String data = server.arg("plain");
    Serial.println(data);

    bool isParseOk = json2patterns(data);

    if (isParseOk && numPatterns > 0) {
      ws2812fx.stop();
      ws2812fx.clear();
      ws2812fx.resetSegments();

      saveToEEPROM();

      currentPattern = 0;
      lastTime = 0;
      ws2812fx.start();
    }

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", "{\"status\":200, \"message\":\"OK\"}");
  });
}

#define EEPROM_MAGIC_NUMBER 0x010e0d05
void saveToEEPROM() {
  Serial.println("saving to EEPROM");
  EEPROM.put(sizeof(int) * 0, (int)EEPROM_MAGIC_NUMBER);
  EEPROM.put(sizeof(int) * 1, (int)ws2812fx.getPin());
  EEPROM.put(sizeof(int) * 2, (int)ws2812fx.getLength());
  EEPROM.put(sizeof(int) * 3, (int)numPatterns);
  EEPROM.put(sizeof(int) * 4, patterns);
  EEPROM.commit(); // for ESP8266 (comment out if using an Arduino)
}

void restoreFromEEPROM() {
  int magicNumber = 0;
  int pin;
  int length;
  EEPROM.get(sizeof(int) * 0, magicNumber);
  if (magicNumber == EEPROM_MAGIC_NUMBER) {
    Serial.println("restoring from EEPROM");
    EEPROM.get(sizeof(int) * 1, pin);
    if (ws2812fx.getPin() != pin) {
      ws2812fx.setPin(pin);
    }
    EEPROM.get(sizeof(int) * 2, length);
    if (ws2812fx.getLength() != length) {
      ws2812fx.setLength(length);
    }
    EEPROM.get(sizeof(int) * 3, numPatterns);
    EEPROM.get(sizeof(int) * 4, patterns);
  }
}

int modeName2Index(const char* name) {
  for (int i = 0; i < ws2812fx.getModeCount(); i++) {
    if (strcmp_P(name, (PGM_P)ws2812fx.getModeName(i)) == 0) {
      return i;
    }
  }
  return 0;
}

#if ARDUINOJSON_VERSION_MAJOR == 5
//#pragma message("Compiling for ArduinoJson v5")
bool json2patterns(String &json) {
  DynamicJsonBuffer jsonBuffer(2000);
  JsonObject& deviceJson = jsonBuffer.parseObject(json);
  if (deviceJson.success()) {
    ws2812fx.setPin(deviceJson["dataPin"]);
    ws2812fx.setLength(deviceJson["numLeds"]);

    JsonArray& patternsJson = deviceJson["patterns"];
    if (patternsJson.size() > 0 ) {
      numPatterns = 0;
      for (int i = 0; i < patternsJson.size(); i++) {
        JsonObject& patt = patternsJson[i];
//      bool isEnabled = patt["isEnabled"];
//      if (! isEnabled) continue; // disabled patterns are not stored

        JsonArray& segmentsJson = patt["segments"];
        if (segmentsJson.size() == 0 ) continue;

        patterns[numPatterns].brightness = patt["brightness"];
        patterns[numPatterns].duration = patt["duration"];

        patterns[numPatterns].numSegments = segmentsJson.size();
        for (int j = 0; j < segmentsJson.size(); j++) {
          JsonObject& seg = segmentsJson[j];
          //seg.printTo(Serial);Serial.println();
          int start = seg["start"];
          if (start < 0 || start >= ws2812fx.getLength()) start = 0;
          patterns[numPatterns].segments[j].start = start;

          int stop = seg["stop"];
          if (stop < 0 || stop >= ws2812fx.getLength()) stop = ws2812fx.getLength() - 1;
          patterns[numPatterns].segments[j].stop = stop;

          if (seg["mode"].is<unsigned int>()) { // seg["mode"] can be a mode number or a mode name
            patterns[numPatterns].segments[j].mode = seg["mode"];
          } else {
            patterns[numPatterns].segments[j].mode = modeName2Index(seg["mode"]);
          }

          int speed = seg["speed"];
          if (speed < SPEED_MIN || speed >= SPEED_MAX) speed = 1000;
          patterns[numPatterns].segments[j].speed = speed;

          patterns[numPatterns].segments[j].options = 0;
          bool reverse = seg["reverse"];
          if (reverse) patterns[numPatterns].segments[j].options |= REVERSE;

          bool gamma = seg["gamma"];
          if (gamma) patterns[numPatterns].segments[j].options |= GAMMA;

          int fadeRate = seg["fadeRate"];
          if (fadeRate > 0) patterns[numPatterns].segments[j].options |= (fadeRate & 0x7) << 4;

          int size = seg["size"];
          if (size > 0) patterns[numPatterns].segments[j].options |= (size & 0x3) << 1;

          JsonArray& colors = seg["colors"]; // the web interface sends three color values
          // convert colors from strings ('#ffffff') to uint32_t
          patterns[numPatterns].segments[j].colors[0] = strtoul(colors[0].as<char*>() + 1, 0, 16);
          patterns[numPatterns].segments[j].colors[1] = strtoul(colors[1].as<char*>() + 1, 0, 16);
          patterns[numPatterns].segments[j].colors[2] = strtoul(colors[2].as<char*>() + 1, 0, 16);
        }
        numPatterns++;
        if (numPatterns >= MAX_NUM_PATTERNS) break;
      }
    } else {
      Serial.println(F("JSON contains no pattern data"));
      return false;
    }
  } else {
    Serial.println(F("Could not parse JSON payload"));
    return false;
  }
  return true;
}
#endif

#if ARDUINOJSON_VERSION_MAJOR == 6
//#pragma message("Compiling for ArduinoJson v6")
bool json2patterns(String &json) {
  // in ArduinoJson v6 a DynamicJsonDocument does not expand as needed
  // like it did in ArduinoJson v5. So rather then try to compute the
  // optimum heap size, we'll just allocated a bunch of space on the
  // heap and hope it's enough.
  int freeHeap = ESP.getFreeHeap();
  DynamicJsonDocument doc(freeHeap - 3096); // allocate all of the available heap except 3kB
  DeserializationError error = deserializeJson(doc, json);
  if (!error) {
    JsonObject deviceJson = doc.as<JsonObject>();
    ws2812fx.setPin(deviceJson["dataPin"]);
    ws2812fx.setLength(deviceJson["numLeds"]);

    JsonArray patternsJson = deviceJson["patterns"];
    if (patternsJson.size() > 0 ) {
      numPatterns = 0;
      for (size_t i = 0; i < patternsJson.size(); i++) {
        JsonObject patt = patternsJson[i];
//      bool isEnabled = patt["isEnabled"];
//      if (! isEnabled) continue; // disabled patterns are not stored

        JsonArray segmentsJson = patt["segments"];
        if (segmentsJson.size() == 0 ) continue;

        patterns[numPatterns].brightness = patt["brightness"];
        patterns[numPatterns].duration = patt["duration"];

        patterns[numPatterns].numSegments = segmentsJson.size();
        for (size_t j = 0; j < segmentsJson.size(); j++) {
          JsonObject seg = segmentsJson[j];
//seg.printTo(Serial);Serial.println();

          int start = seg["start"];
          if (start < 0 || start >= ws2812fx.getLength()) start = 0;
          patterns[numPatterns].segments[j].start = start;

          int stop = seg["stop"];
          if (stop < 0 || stop >= ws2812fx.getLength()) stop = ws2812fx.getLength() - 1;
          patterns[numPatterns].segments[j].stop = stop;

          if (seg["mode"].is<unsigned int>()) { // seg["mode"] can be a mode number or a mode name
            patterns[numPatterns].segments[j].mode = seg["mode"];
          } else {
            patterns[numPatterns].segments[j].mode = modeName2Index(seg["mode"]);
          }

          int speed = seg["speed"];
          if (speed < SPEED_MIN || speed >= SPEED_MAX) speed = 1000;
          patterns[numPatterns].segments[j].speed = speed;

          patterns[numPatterns].segments[j].options = 0;
          bool reverse = seg["reverse"];
          if (reverse) patterns[numPatterns].segments[j].options |= REVERSE;

          bool gamma = seg["gamma"];
          if (gamma) patterns[numPatterns].segments[j].options |= GAMMA;

          int fadeRate = seg["fadeRate"];
          if (fadeRate > 0) patterns[numPatterns].segments[j].options |= (fadeRate & 0x7) << 4;

          int size = seg["size"];
          if (size > 0) patterns[numPatterns].segments[j].options |= (size & 0x3) << 1;

          JsonArray colors = seg["colors"]; // the web interface sends three color values
          // convert colors from strings ('#ffffff') to uint32_t
          patterns[numPatterns].segments[j].colors[0] = strtoul(colors[0].as<const char*>() + 1, 0, 16);
          patterns[numPatterns].segments[j].colors[1] = strtoul(colors[1].as<const char*>() + 1, 0, 16);
          patterns[numPatterns].segments[j].colors[2] = strtoul(colors[2].as<const char*>() + 1, 0, 16);
        }
        numPatterns++;
        if (numPatterns >= MAX_NUM_PATTERNS)  break;
      }
    } else {
      Serial.println(F("JSON contains no pattern data"));
      return false;
    }
  } else {
    Serial.print(F("Could not parse JSON payload: "));
    Serial.println(error.c_str());
    return false;
  }
  return true;
}
#endif
