#!/bin/zsh

# simple test script that verifies all the WS2812FX example sketches.
# note: behind the scenes this script runs the Arduino IDE, so you will see
# the IDE's splash screen displayed during each individual test.
# also note: this script takes a long time to run. better to pip the output
# a file and go get a cup of coffee:
#   ./tester.zsh > tester.txt

# add the Arduino application to $PATH
export PATH=$PATH:/Applications/Arduino.app/Contents/MacOS

# test that we can execute the Arduino command
#Arduino --version

# board aliases (fully qualified board name)
alias -g leonardo-board="arduino:avr:leonardo"
alias -g esp8266-board="esp8266:esp8266:nodemcuv2"
alias -g esp32-board="esp32:esp32:esp32doit-devkit-v1"
alias -g rp2040-board="arduino:mbed_rp2040:pico"

# port aliases
alias -g leonardo-port="/dev/cu.usbmodem14101 (Arduino Leonardo)"
alias -g esp8266-port="/dev/cu.usbserial-0001"
alias -g esp32-port="/dev/cu.usbserial-0001"
alias -g rp2040-port="/dev/cu.usbmodem14101 (Raspberry Pi Pico)"

# example verify command:
# arduino --board leonardo-board --verify ws2812fx_segments/ws2812fx_segments.ino

# example upload command:
#arduino --board leonardo-board --port leonardo-port --upload ws2812fx_segments/ws2812fx_segments.ino

# create a list of basic example sketches that can be verified for all boards
basic_sketches=(
  'auto_mode_cycle/auto_mode_cycle.ino',
  'external_trigger/external_trigger.ino',
  'serial_control/serial_control.ino',
  'ws2812fx_audio_reactive/ws2812fx_audio_reactive.ino',
  'ws2812fx_custom_effect/ws2812fx_custom_effect.ino',
  'ws2812fx_custom_effect2/ws2812fx_custom_effect2.ino',
  'ws2812fx_custom_FastLED/ws2812fx_custom_FastLED.ino',
  'ws2812fx_limit_current/ws2812fx_limit_current.ino',
  'ws2812fx_matrix/ws2812fx_matrix.ino',
  'ws2812fx_msgeq7/ws2812fx_msgeq7.ino',
  'ws2812fx_overlay/ws2812fx_overlay.ino',
  'ws2812fx_segment_sequence/ws2812fx_segment_sequence.ino',
  'ws2812fx_segments/ws2812fx_segments.ino',
  'ws2812fx_spi/ws2812fx_spi.ino',
  'ws2812fx_transitions/ws2812fx_transitions.ino',
  'ws2812fx_virtual_strip/ws2812fx_virtual_strip.ino'
)

# run verify command for all sketches for the Arduino Leonardo board
for ((i = 1; i <= $#basic_sketches; i++)) {
  echo "\nVerifing" $basic_sketches[i] for Arduino Leonardo
  arduino --board leonardo-board --verify $basic_sketches[i] 2>/dev/null; echo "exit status" $?
}

# run verify command for all sketches for the ESP8266 board
for ((i = 1; i <= $#basic_sketches; i++)) {
  echo "\nVerifing" $basic_sketches[i] for ESP8266
  arduino --board esp8266-board --verify $basic_sketches[i] 2>/dev/null; echo "exit status" $?
}

# run verify command for all sketches for the ESP32 board
for ((i = 1; i <= $#basic_sketches; i++)) {
  echo "\nVerifing" $basic_sketches[i] for ESP32
  arduino --board esp32-board --verify $basic_sketches[i] 2>/dev/null; echo "exit status" $?
}

# run verify command for all sketches for the RP2040 board
for ((i = 1; i <= $#basic_sketches; i++)) {
  echo "\nVerifing" $basic_sketches[i] for RP2040
  arduino --board rp2040-board --verify $basic_sketches[i] 2>/dev/null; echo "exit status" $?
}

# create a list of example sketches that use WiFi to be verified only on ESP boards
wifi_sketches=(
  'esp8266_webinterface/esp8266_webinterface.ino',
  'ws2812fx_alexa/ws2812fx_alexa.ino',
  'ws2812fx_patterns_web/ws2812fx_patterns_web.ino',
  'ws2812fx_segments_OTA/ws2812fx_segments_OTA.ino',
  'ws2812fx_segments_web/ws2812fx_segments_web.ino',
  'ws2812fx_extData/ws2812fx_extData.ino'
)

# run verify command for all sketches for the ESP8266 board
for ((i = 1; i <= $#wifi_sketches; i++)) {
  echo "\nVerifing" $wifi_sketches[i] for ESP8266
  arduino --board esp8266-board --verify $wifi_sketches[i] 2>/dev/null; echo "exit status" $?
}

# run verify command for all sketches for the ESP32 board
for ((i = 1; i <= $#wifi_sketches; i++)) {
  echo "\nVerifing" $wifi_sketches[i] for ESP32
  arduino --board esp32-board --verify $wifi_sketches[i] 2>/dev/null; echo "exit status" $?
}

# the ws2812fx_soundfx example sketch uses the ESP8266Audio, which doesn't seem to support
# the ESP32 very well, so test that separately
echo "\nVerifing ws2812fx_soundfx/ws2812fx_soundfx.ino for ESP8266"
arduino --board esp8266-board --verify ws2812fx_soundfx/ws2812fx_soundfx.ino 2>/dev/null; echo "exit status" $?

# the ws2812fx_dma example sketch is written to work only on ESP8266 boards, so test that separately
echo "\nVerifing ws2812fx_dma/ws2812fx_dma.ino for ESP8266"
arduino --board esp8266-board --verify ws2812fx_dma/ws2812fx_dma.ino 2>/dev/null; echo "exit status" $?
