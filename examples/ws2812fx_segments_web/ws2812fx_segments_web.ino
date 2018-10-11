/*
  WS2812FX segmented led strip web demo.
  
  Keith Lord - 2017
  
  FEATURES
    * example of a web application to control segments of a strip of WS2812 LEDs


  LICENSE

  The MIT License (MIT)

  Copyright (c) 2017  Keith Lord 

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
  2017-10-02 initial version
  2017-10-08 added web interface
  
*/

#include <WS2812FX.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

extern const char index_html[];

#define LED_PIN   D1 // digital pin used to drive the LED strip (esp8266)
#define LED_COUNT 30 // number of LEDs on the strip

#define WIFI_SSID "xxxxxxxx"     // WiFi network
#define WIFI_PASSWORD "xxxxxxxx" // WiFi network password

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // init WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  Serial.print("Connecting to " WIFI_SSID);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\r\nPoint your browser to ");
  Serial.println(WiFi.localIP());

  /* init OTA */
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

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

  /* init web server */
  // return the index.html file
  server.on("/", [](){
    server.send_P(200, "text/html", index_html);
  });

  // send the segment info in JSON format
  server.on("/getsegments", [](){
    DynamicJsonBuffer jsonBuffer(1000);
    JsonObject& root = jsonBuffer.createObject();
    root["pin"] = ws2812fx.getPin();
    root["numPixels"] = ws2812fx.numPixels();
    root["brightness"] = ws2812fx.getBrightness();
    root["numSegments"] = ws2812fx.getNumSegments();
    JsonArray& jsonSegments = root.createNestedArray("segments");

    WS2812FX::segment* segments = ws2812fx.getSegments();
    for(int i=0; i<ws2812fx.getNumSegments(); i++) {
      WS2812FX::segment seg = segments[i];
      JsonObject& jsonSegment = jsonBuffer.createObject();
      jsonSegment["start"] = seg.start;
      jsonSegment["stop"] = seg.stop;
      jsonSegment["mode"] = seg.mode;
      jsonSegment["speed"] = seg.speed;
      jsonSegment["reverse"] = seg.options & REVERSE ? true: false;
      JsonArray& jsonColors = jsonSegment.createNestedArray("colors");
      jsonColors.add(seg.colors[0]); // the web interface expects three color values
      jsonColors.add(seg.colors[1]);
      jsonColors.add(seg.colors[2]);
      jsonSegments.add(jsonSegment);
    }
    // root.printTo(Serial);

    int bufferSize = root.measureLength() + 1;
    char *json = (char*)malloc(sizeof(char) * (bufferSize));
    root.printTo(json, sizeof(char) * bufferSize);
    server.send(200, "application/json", json);
    free(json);
  });

  // receive the segment info in JSON format and setup the WS2812 strip
  server.on("/setsegments", HTTP_POST, [](){
    String data = server.arg("plain");
    data = data.substring(1, data.length() - 1); // remove the surrounding quotes
    data.replace("\\\"", "\""); // replace all the backslash quotes with just quotes
//Serial.println(data);
    DynamicJsonBuffer jsonBuffer(1000);
    JsonObject& root = jsonBuffer.parseObject(data);
    if (root.success()) {
      ws2812fx.stop();
      ws2812fx.setLength(root["numPixels"]);
      ws2812fx.stop(); // reset strip again in case length was increased
      ws2812fx.setBrightness(root["brightness"]);
      ws2812fx.setNumSegments(1); // reset number of segments
      JsonArray& segments = root["segments"];
      for (int i=0; i<segments.size(); i++){
        JsonObject& seg = segments[i];
        JsonArray& colors = seg["colors"];
        // the web interface sends three color values
        uint32_t _colors[NUM_COLORS] = {colors[0], colors[1], colors[2]};
        bool reverse = seg["reverse"];
        ws2812fx.setSegment(i, seg["start"], seg["stop"], seg["mode"], _colors, seg["speed"], reverse ? REVERSE : NO_OPTIONS);
      }
      ws2812fx.start();
    }
    server.send(200, "text/plain", "OK");
  });

  // send the WS2812 mode info
  server.on("/getmodes", [](){
    DynamicJsonBuffer jsonBuffer(1000);
    JsonArray& root = jsonBuffer.createArray();
    for(uint8_t i=0; i < ws2812fx.getModeCount(); i++) {
      root.add(ws2812fx.getModeName(i));
    }

    int bufferSize = root.measureLength() + 1;
    char *json = (char*)malloc(sizeof(char) * (bufferSize));
    root.printTo(json, sizeof(char) * bufferSize);
    server.send(200, "application/json", json);
    free(json);
  });

  server.onNotFound([](){
    server.send(404, "text/plain", "Page not found");
  });

  // start the web server
  server.begin();

  // init LED strip with a default segment
  ws2812fx.init();
  ws2812fx.setBrightness(127);
  // parameters: seg index, start led, stop led, mode, color, speed, reverse
  ws2812fx.setSegment(0, 0, LED_COUNT-1, FX_MODE_SCAN, RED, 1000, false);
  ws2812fx.start();
}

void loop() {
  ws2812fx.service();
  server.handleClient();
  ArduinoOTA.handle();
}
