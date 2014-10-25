#include <Arduino.h>

boolean ledState;

//  Arduino Due           || Teensy 3.1             || Teensy 3.0             || Arduino Pro Mini
#if defined (__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK20DX128__) || defined(__AVR_ATmega328P__) 
const int LED_OUT          = 13; // Arduino onboard LED; HIGH (= ON) while recording

void ledOff()
{
  digitalWrite(LED_OUT, LOW);
  ledState = false;
}

void ledOn()
{
  digitalWrite(LED_OUT, HIGH);
  ledState = true;
}

void setupLED()
{
  pinMode(LED_OUT, OUTPUT);
  ledOff();
}
//    Arduino Pro Micro
#elif defined(__AVR_ATmega32U4__)
void ledOff()
{
  TXLED0;
  RXLED0;
  ledState = false;
}

void ledOn()
{
  TXLED1;
  RXLED1;
  ledState = true;
}

void setupLED()
{
  ledOff();
}
//    GR-KURUMI
#elif defined(REL_GR_KURUMI)
const int LED_OUT_R        = 22; // GR-KURUMI RGB-LED; active LOW
const int LED_OUT_G        = 23;
const int LED_OUT_B        = 24;

void ledOff()
{
  // white
  pinMode(LED_OUT_R, INPUT);
  pinMode(LED_OUT_G, INPUT);
  pinMode(LED_OUT_B, INPUT);
  ledState = false;
}

void ledOn()
{
  pinMode(LED_OUT_R, OUTPUT);
  pinMode(LED_OUT_G, OUTPUT);
  pinMode(LED_OUT_B, OUTPUT);
  // green
  digitalWrite(LED_OUT_R, HIGH);
  digitalWrite(LED_OUT_G, LOW);
  digitalWrite(LED_OUT_B, HIGH);
  ledState = true;
}

void setupLED()
{
  ledOff();
}
#else
#error CPU not supported
#endif
