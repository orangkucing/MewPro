// Visible Light Sensor interface for Underwater Scuba & Dive - Waterproof Green Laser Pointers
//
// Microsemi LX1971:
//   peak sensitivity 520nm (Green)
//
// Schematics to connect LX1971 and Arduino Pro Mini is "Figure 1" in the following PDF:
//   http://www.microsemi.com/document-portal/doc_download/132243-lx1971-datasheet
// where R1 = 22k, R2 = 4k7, C1 = 100μF, connecting SRC to A7.
//
// Note: C1 is to slow down the response time of LX1971 and to increase stability.
// Should be set as the product R2·C1 is nearly equal to 500ms.

#ifdef USE_LIGHT_SENSOR

void lightOnCommand()
{
  if (!ledState) {
    startRecording();
  } else {
    stopRecording();
  }
}

void lightOffCommand()
{
  delay(1000);
}

void setupLightSensor()
{
}

void checkLightSensor()
{
//  const int threshold = 360; // 15000lux = SRC current 150μA (reached the saturation at load resistor R2 = 4k7) 
  const int threshold = 228; // 2500lux = SRC current 40μA


  static unsigned long lastDebounceTime = 0;
  static int lastButtonState = 0;
  static int buttonState;
  int val = analogRead(LIGHT_SENSOR_PIN);

#if 0
  static int maximum = -1;
  if (val > maximum) {
    maximum = val;
    if (debug) {
      Serial_println(maximum); // For debug
    }
  }
#endif

  int reading = (val > threshold);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if (millis() - lastDebounceTime > 100) {
    if (reading != buttonState) {
      if (reading != 0) {
        lightOnCommand();
      } else {
        lightOffCommand();
      }
      buttonState = reading;
    }
  }
  lastButtonState = reading;
}

#else

void setupLightSensor()
{
}

void checkLightSensor()
{
}

#endif
