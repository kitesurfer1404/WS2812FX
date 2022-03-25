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

#define DYNAMIC_JSON_DOCUMENT_SIZE 4096 /* used by AsyncJson. Default 1024 bytes is too small */

#include <WS2812FX.h>
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#define VERSION "2.4.0"

uint8_t  dataPin = 14; // default digital pin used to drive the LED strip
uint16_t numLeds = 30; // default number of LEDs on the strip

#define WIFI_SSID "xxxxxxxx"     // WiFi network
#define WIFI_PASSWORD "xxxxxxxx" // WiFi network password
#define HTTP_PORT 80

#define MAX_NUM_PATTERNS 8

typedef struct Pattern { // 208 bytes/pattern
  int duration;
  uint8_t brightness;
  uint8_t numSegments;
  WS2812FX::segment segments[MAX_NUM_SEGMENTS];
} pattern;

// setup a default patterns array
Pattern patterns[MAX_NUM_PATTERNS] = {
  { 300, 32, 1, { // duration, brightness, numSegments
      // first, last, speed, mode, options, colors[]
      {0, (uint16_t)(numLeds - 1), 5000, FX_MODE_STATIC, NO_OPTIONS, {BLUE,  BLACK, BLACK}}
    }
  }
};
int numPatterns = 1;
int currentPattern = 0;
unsigned long lastTime = 0;

WS2812FX ws2812fx = WS2812FX(numLeds, dataPin, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(HTTP_PORT);

void (*customAuxFunc[])(void) { // define custom auxiliary functions here
  [] { Serial.println("running customAuxFunc[0]"); },
  [] { Serial.println("running customAuxFunc[1]"); },
  [] { Serial.println("running customAuxFunc[2]"); }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\r\n");

  // init LED strip with a default segment
  ws2812fx.init();
  ws2812fx.setBrightness(64);
  ws2812fx.setSegment(0, 0, numLeds - 1, FX_MODE_STATIC, GREEN, 1000);
  ws2812fx.start();
  Serial.println("LED strip initialized");

  EEPROM.begin(4096); // for ESP (comment out if using an Arduino)

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


#ifdef ARDUINO_ARCH_ESP8266
  // if not rebooting due to catastrophic error, restore pattern data from eeprom
  struct rst_info  *rstInfo = system_get_rst_info(); // ESP8266
  Serial.print("rstInfo->reason:"); Serial.println(rstInfo->reason);
  Serial.print("ESP.getResetReason():"); Serial.println(ESP.getResetReason());
  if (rstInfo->reason !=  REASON_EXCEPTION_RST) { // not reason 2
    restoreFromEEPROM();
  }
#endif

#ifdef ARDUINO_ARCH_ESP32
  // if esp32 was reset by powering on or uploading a program via the serial port or
  // uploading a program via OTA, restore pattern data from eeprom
  esp_reset_reason_t rstInfo = esp_reset_reason(); // ESP32
  if (rstInfo == ESP_RST_POWERON || rstInfo == ESP_RST_WDT || rstInfo == ESP_RST_SW) {
    restoreFromEEPROM();
  }
#endif

  // config and start the web server
  configServer();
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*"); // add CORS header
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  ws2812fx.service();

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
    ws2812fx.start();
    lastTime = now;
  }
}

void configServer() {

  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Page not found");
  });

  // return the WS2812FX status
  // optionally set the running state or run custom auxiliary functions
  server.on("/status", [](AsyncWebServerRequest * request) {
    if (request->hasParam("running")) {
      AsyncWebParameter* p = request->getParam("running");
      const char* running = p->value().c_str();
      if (strcmp(running, "true") == 0) ws2812fx.start();
      else ws2812fx.stop();
    }

    if (request->hasParam("auxFunc")) {
      AsyncWebParameter* p = request->getParam("auxFunc");
      int auxFuncIndex = atoi(p->value().c_str());
      size_t customAuxFuncSize = sizeof(customAuxFunc) / sizeof(customAuxFunc[0]);
      if (auxFuncIndex >= 0 && (size_t)auxFuncIndex < customAuxFuncSize) {
        customAuxFunc[auxFuncIndex]();
      }
    }

    char status[50] = "{\"version\":\"";
    strcat(status, VERSION);
    strcat(status, "\",\"isRunning\":");
    strcat(status, ws2812fx.isRunning() ? "true" : "false");
    strcat(status, "}");

    request->send(200, "application/json", status);
  });

  // send the WS2812FX mode info in JSON format
  server.on("/getModes", [](AsyncWebServerRequest * request) {
    char modes[1300] = "[";
    for (uint8_t i = 0; i < ws2812fx.getModeCount(); i++) {
      strcat(modes, "\"");
      strcat_P(modes, (PGM_P)ws2812fx.getModeName(i));
      strcat(modes, "\",");
    }
    modes[strlen(modes) - 1] = ']';

    request->send(200, "application/json", modes);
  });

  server.on("/upload", HTTP_OPTIONS, [](AsyncWebServerRequest * request) { // CORS preflight request
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "OK");
    response->addHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
    request->send(response);
  });

  // receive the device info in JSON format and update the pattern data
  AsyncCallbackJsonWebHandler* uploadHandler = new AsyncCallbackJsonWebHandler("/upload",
  [](AsyncWebServerRequest * request, JsonVariant & json) {
    if (request->method() == HTTP_POST) {
      JsonObject jsonObj = json.as<JsonObject>();
      serializeJson(jsonObj, Serial); Serial.println(); // debug

      bool isParseOk = json2patterns(jsonObj);
    if (isParseOk && numPatterns > 0) {
      ws2812fx.stop();
      ws2812fx.clear();
      ws2812fx.resetSegments();

      saveToEEPROM();

      currentPattern = 0;
      lastTime = 0;
      ws2812fx.start();
    }

      AsyncWebServerResponse *response = request->beginResponse(200, "application/json", "{\"status\":200, \"message\":\"OK\"}");
      request->send(response);
    }
  });
  server.addHandler(uploadHandler);
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
    ws2812fx.stop();
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
    ws2812fx.start();
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

#if ARDUINOJSON_VERSION_MAJOR == 6
//#pragma message("Compiling for ArduinoJson v6")
bool json2patterns(JsonObject deviceJson) {
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
  return true;
}
#endif
