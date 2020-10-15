#include <Arduino.h>
#include <FastGPIO.h>
#define APA102_USE_FAST_GPIO
#include <APA102.h>

const long debouncing_time = 1000;
volatile unsigned long last_micros;

const byte modeButton = 2;
volatile byte mode = 8;
const uint8_t dataPin = 11;
const uint8_t clockPin = 12;
const uint16_t ledCount = 19;

APA102<dataPin, clockPin> ledStrip;
rgb_color colors[ledCount];
rgb_color colors_off[ledCount];

uint8_t brightness = 1;
uint16_t color = 1;

void switchMode() {
  if((long)(micros() - last_micros) >= debouncing_time * 1000) {
    mode++;
    last_micros = micros();
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(modeButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(modeButton), switchMode, RISING);
}

/* Converts a color from HSV to RGB.
 * h is hue, as a number between 0 and 360.
 * s is the saturation, as a number between 0 and 255.
 * v is the value, as a number between 0 and 255. */
rgb_color hsvToRgb(uint16_t h, uint8_t s, uint8_t v)
{
    uint8_t f = (h % 60) * 255 / 60;
    uint8_t p = (255 - s) * (uint16_t)v / 255;
    uint8_t q = (255 - f * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t t = (255 - (255 - f) * (uint16_t)s / 255) * (uint16_t)v / 255;
    uint8_t r = 0, g = 0, b = 0;
    switch((h / 60) % 6){
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }
    return rgb_color(r, g, b);
}

void gradientEffect(){
  uint8_t time = millis() >> 2;
  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t x = time - i * 8;
    colors[i] = rgb_color(x, 255 - x, x);
  }
  ledStrip.write(colors, ledCount, brightness);
  delay(10);
}

void rainbowEffect(){
  uint8_t time = millis() >> 4;
  for(uint16_t i = 0; i < ledCount; i++)
  {
    uint8_t p = time - i * 8;
    colors[i] = hsvToRgb((uint32_t)p * 359 / 256, 255, 255);
  }
  ledStrip.write(colors, ledCount, brightness);
  delay(10);
}

void colorEffect(uint16_t h, uint8_t s, uint8_t v){
  for(uint16_t i = 0; i < ledCount; i++)
  {
    colors[i] = hsvToRgb(h, s, v);
  }
  ledStrip.write(colors, ledCount, brightness);
  delay(10);
}

void colorWipeEffect(uint_fast16_t h, uint8_t d){
  for(uint16_t i = 0; i < ledCount; i++)
  {
    colors[i] = hsvToRgb(h, 255, 255);
    ledStrip.write(colors, ledCount, brightness);
    delay(d);
  }
  for(uint16_t i = 0; i < ledCount; i++)
  {
    colors[i] = hsvToRgb(0, 0, 0);
    ledStrip.write(colors, ledCount, brightness);
    delay(d);
  }
}

void strobeEffect(uint16_t h){
  for(uint16_t i = 0; i < ledCount; i++)
  {
    colors[i] = hsvToRgb(h, 255, 255);
  }
  for(uint16_t i = 0; i < ledCount; i++)
  {
    colors_off[i] = hsvToRgb(0, 0, 0);
  }
  for(uint8_t i = 0; i < 10; i++)
  {
    ledStrip.write(colors, ledCount, brightness);
    delay(50);
    ledStrip.write(colors_off, ledCount, brightness);
    delay(50);
  }
  delay(1000);
}

void sparkleEffect(uint16_t h){
  uint16_t pixel = random(ledCount);
  colors[pixel] = hsvToRgb(h, 255, 255);
  ledStrip.write(colors, ledCount, brightness);
  colors[pixel] = hsvToRgb(0, 0, 0);
}

void loop()
{
  switch (mode)
  {
  case 1:
    Serial.println("Entered color mode");
    while(mode == 1){
      // Color mode
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      color = map(analogRead(A1), 0, 1023, 0, 360);
      colorEffect(color, 255, 255);
    }

  case 2:
    Serial.println("Entered white mode");
    while(mode == 2){
      //White mode
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      colorEffect(0, 0, 255);
    }

  case 3:
    Serial.println("Entered rainbow mode");
    while(mode == 3){
      // Rainbow mode
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      rainbowEffect();
    }

  case 4:
    Serial.println("Entered gradient mode");
    while(mode == 4){
      // Gradient mode
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      gradientEffect();
    }

  case 5:
    Serial.println("Entered color wipe mode");
    while(mode == 5){
      // Color wipe mode
      color = map(analogRead(A1), 0, 1023, 0, 360);
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      colorWipeEffect(color, 50);
    }

  case 6:
    Serial.println("Entered strobe mode");
    while(mode == 6){
      // Strobe mode
      color = map(analogRead(A1), 0, 1023, 0, 360);
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      strobeEffect(color);
    }

  case 7:
    Serial.println("Entered sparkle mode");
    while(mode == 7){
      // Sparkle mode
      color = map(analogRead(A1), 0, 1023, 0, 360);
      brightness = map(analogRead(A0), 0, 1023, 0, 31);
      sparkleEffect(color);
    }
  
  default:
    mode = 1;
    break;
  }
}