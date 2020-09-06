#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <Arduino.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <StreamString.h>
#include <BlynkSimpleEsp8266.h>
#include <WS2812FX.h>

#define BLYNK_PRINT Serial
#define LED_COUNT 150
#define LED_PIN D2

#define MyApiKey "" //Api key sinric.com
#define MyWifiSSID "" //Wifi ssid
#define MyWifiPassword "" //Wifi password
#define MyBlynkAuth "" //Blynk auth key
#define MyLedStripId "" //Light id, sinric.com
#define HEARTBEAT_INTERVAL 300000

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;
WiFiClient client;
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void printColor(int r, int g, int b){
  Serial.println((String)"\nRed:"+r+"\nGreen:"+g+"\nBlue:"+b);
}

void onOff(String deviceId, bool statusLed) {
  if (deviceId == MyLedStripId){
    if (statusLed) {
      ws2812fx.start();
    }else{
      ws2812fx.stop();
    }
    Blynk.virtualWrite(V4, statusLed);
  }   
}

void colorAbsolute(String deviceId, int decimalColor){
  if (deviceId == MyLedStripId){  
    String hexstring =  String(decimalColor, HEX);
    hexstring = "#" + hexstring;
    int number = (int) strtol( &hexstring[1], NULL, 16);
    int r = number >> 16;
    int g = number >> 8 & 0xFF;
    int b = number & 0xFF;
    ws2812fx.setColor(g, r, b);
    Blynk.virtualWrite(V4, true);
    Blynk.virtualWrite(V5, r, g, b);
    ws2812fx.start();
  }
}

void brightnessAbsolute(String deviceId, int brilho){
  if (deviceId == MyLedStripId){
    brilho = map(brilho, 0, 100, 0, 255);
    ws2812fx.setBrightness(brilho);
    Blynk.virtualWrite(V3, brilho);
  }
}

String formatHEX(String cor){
  if(cor.length() == 0){
    cor = "00"; 
  }
  else if(cor.length()==1){
    cor = "0" + cor;
  }
  return cor;
}

String convertRGBtoHEX(int r, int g, int b){
  String redStr =  formatHEX(String(r, HEX));
  String greenStr =  formatHEX(String(g, HEX));
  String blueStr =  formatHEX(String(b, HEX));
  String hexStr = redStr + greenStr + blueStr;
  hexStr.toUpperCase();
  return hexStr;
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[WSc] get text: %s\n", payload);

#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "action.devices.commands.OnOff") { // Switch 
          String value = json ["value"]["on"];
          onOff(deviceId, value == "true");
        }
        else if (action  == "action.devices.commands.ColorAbsolute") {
          String value = json ["value"]["color"]["spectrumRGB"];
          colorAbsolute(deviceId, value.toInt());
        }
        else if (action  == "action.devices.commands.BrightnessAbsolute") {
          String value = json ["value"]["brightness"];
          brightnessAbsolute(deviceId, value.toInt());
        }
        else if (action == "test") {
          Serial.println("Recebendo o comando de teste da sinric.com");
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
    default: break;
  }
}

void setupWifi(){
  WiFiMulti.addAP(MyWifiSSID, MyWifiPassword);
  Serial.println();
  Serial.print("Conectando ao Wifi: ");
  Serial.println(MyWifiSSID);  

  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi conectado. ");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  }
}

void setupWebSocket(){
  webSocket.begin("iot.sinric.com", 80, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", MyApiKey);
  webSocket.setReconnectInterval(5000);
}

void setupWs2812fx(){
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(200);
  ws2812fx.setColor(255, 255, 255);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();
}

void setupBlynk(){
  Blynk.begin(MyBlynkAuth, MyWifiSSID, MyWifiPassword);
}

void setup() {
  Serial.begin(9600);
  setupWifi();
  setupWs2812fx();
  setupBlynk();
  setupWebSocket();
}

void loop() {
  Blynk.run();
  webSocket.loop();
  ws2812fx.service();

  if(isConnected) {
      uint64_t now = millis();
     if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }
}
BLYNK_WRITE(V0) {//Notification
}

BLYNK_WRITE(V1) {//Speed
  ws2812fx.setSpeed(param.asInt()*10);
}

BLYNK_WRITE(V2) {//Efeitos
  ws2812fx.setMode(param.asInt() - 1);
}

BLYNK_WRITE(V3) {//Brilho
  ws2812fx.setBrightness(param.asInt());
}

BLYNK_WRITE(V4) {//OnOff
  if (param.asInt()) {
    ws2812fx.start();
  }else{
    ws2812fx.stop();
  }
}

BLYNK_WRITE(V5) {//Cor GRB
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  ws2812fx.setColor(g, r, b);
  Blynk.virtualWrite(V6, convertRGBtoHEX(r, g, b));
}

BLYNK_WRITE(V6){//Color HEX
  String hexstring = param.asStr();
  hexstring = "#" + hexstring;  
  int number = (int) strtol( &hexstring[1], NULL, 16);
  int r = number >> 16;
  int g = number >> 8 & 0xFF;
  int b = number & 0xFF;
  
  ws2812fx.setColor(g, r, b);
  Blynk.virtualWrite(V5, r, g, b);
}
