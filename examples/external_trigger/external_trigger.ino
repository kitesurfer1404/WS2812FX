#include <WS2812FX.h>

#define LED_COUNT 150
#define LED_PIN 13

#define ANALOG_PIN A0
#define ANALOG_THRESHOLD 512

#define TIMER_MS 3000

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

unsigned long last_trigger = 0;
unsigned long now = 0;

void setup() {
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setMode(FX_MODE_RANDOM_COLOR);
}

void loop() {
  now = millis();

  ws2812fx.service();

  // trigger on a regular basis
  if(now - last_trigger > TIMER_MS) {
    ws2812fx.trigger();
    last_trigger = now;
  }

  // trigger, if analog value is above threshold
  // this comes in handy, when using a microphone on analog input
  if(analogRead(ANALOG_PIN) > ANALOG_THRESHOLD) {
    ws2812fx.trigger();
  }
}