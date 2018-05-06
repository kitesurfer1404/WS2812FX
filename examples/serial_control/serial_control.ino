#define REDUCED_MODES // sketch is too big for Arduino w/32k flash, so invoke reduced modes
#include <WS2812FX.h>

#define LED_COUNT 30
#define LED_PIN 5

// ESP boards generate a "section type conflict with __c" error when
// putting non-global strings in PROGMEM. i.e. can't use F() for local strings.
#if defined(__AVR__) // Arduino
  #define STRING(x) F(x)
#else // ESP
  #define STRING(x) x
#endif

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

String cmd = "";               // String to store incoming serial commands
boolean cmd_complete = false;  // whether the command string is complete


void setup() {
  Serial.begin(115200);
  ws2812fx.init();
  ws2812fx.setBrightness(30);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.start();

  printModes();
  printUsage();
}

void loop() {
  ws2812fx.service();

  // On Atmega32U4 based boards (leonardo, micro) serialEvent is not called
  // automatically when data arrive on the serial RX. We need to do it ourself
  #if defined(__AVR_ATmega32U4__) || defined(ESP8266)
  serialEvent();
  #endif

  if(cmd_complete) {
    process_command();
  }
}

/*
 * Checks received command and calls corresponding functions.
 */
void process_command() {
  if(cmd == STRING("b+")) { 
    ws2812fx.increaseBrightness(25);
    Serial.print(STRING("Increased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if(cmd == STRING("b-")) {
    ws2812fx.decreaseBrightness(25); 
    Serial.print(STRING("Decreased brightness by 25 to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if(cmd.startsWith(STRING("b "))) { 
    uint8_t b = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setBrightness(b);
    Serial.print(STRING("Set brightness to: "));
    Serial.println(ws2812fx.getBrightness());
  }

  if(cmd == STRING("s+")) { 
//  ws2812fx.increaseSpeed(10);
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 1.2);
    Serial.print(STRING("Increased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if(cmd == STRING("s-")) {
//  ws2812fx.decreaseSpeed(10);
    ws2812fx.setSpeed(ws2812fx.getSpeed() * 0.8);
    Serial.print(STRING("Decreased speed by 20% to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if(cmd.startsWith(STRING("s "))) {
    uint16_t s = (uint16_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setSpeed(s); 
    Serial.print(STRING("Set speed to: "));
    Serial.println(ws2812fx.getSpeed());
  }

  if(cmd.startsWith(STRING("m "))) { 
    uint8_t m = (uint8_t) cmd.substring(2, cmd.length()).toInt();
    ws2812fx.setMode(m);
    Serial.print(STRING("Set mode to: "));
    Serial.print(ws2812fx.getMode());
    Serial.print(" - ");
    Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
  }

  if(cmd.startsWith(STRING("c "))) { 
    uint32_t c = (uint32_t) strtol(&cmd.substring(2, cmd.length())[0], NULL, 16);
    ws2812fx.setColor(c); 
    Serial.print(STRING("Set color to: "));
    Serial.print(STRING("0x"));
    if(ws2812fx.getColor() < 0x100000) { Serial.print(STRING("0")); }
    if(ws2812fx.getColor() < 0x010000) { Serial.print(STRING("0")); }
    if(ws2812fx.getColor() < 0x001000) { Serial.print(STRING("0")); }
    if(ws2812fx.getColor() < 0x000100) { Serial.print(STRING("0")); }
    if(ws2812fx.getColor() < 0x000010) { Serial.print(STRING("0")); }
    Serial.println(ws2812fx.getColor(), HEX);
  }

  cmd = "";              // reset the commandstring
  cmd_complete = false;  // reset command complete
}

/*
 * Prints a usage menu.
 */
void printUsage() {
  Serial.println(STRING("Usage:"));
  Serial.println();
  Serial.println(STRING("m <n> \t : select mode <n>"));
  Serial.println();
  Serial.println(STRING("b+ \t : increase brightness"));
  Serial.println(STRING("b- \t : decrease brightness"));
  Serial.println(STRING("b <n> \t : set brightness to <n>"));
  Serial.println();
  Serial.println(STRING("s+ \t : increase speed"));
  Serial.println(STRING("s- \t : decrease speed"));
  Serial.println(STRING("s <n> \t : set speed to <n>"));
  Serial.println();
  Serial.println(STRING("c 0x007BFF \t : set color to 0x007BFF"));
  Serial.println();
  Serial.println();
  Serial.println(STRING("Have a nice day."));
  Serial.println(STRING("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
  Serial.println();
}


/*
 * Prints all available WS2812FX blinken modes.
 */
void printModes() {
  Serial.println(STRING("Supporting the following modes: "));
  Serial.println();
  for(int i=0; i < ws2812fx.getModeCount(); i++) {
    Serial.print(i);
    Serial.print(STRING("\t"));
    Serial.println(ws2812fx.getModeName(i));
  }
  Serial.println();
}


/*
 * Reads new input from serial to cmd string. Command is completed on \n
 */
void serialEvent() {
  while(Serial.available()) {
    char inChar = (char) Serial.read();
    if(inChar == '\n') {
      cmd_complete = true;
    } else {
      cmd += inChar;
    }
  }
// ESP8266 doesn't terminate the command with "/n", so manually mark the command complete
#if defined(ESP8266)
  cmd_complete = true;
#endif
}
