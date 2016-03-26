/* 
 * //////////////////////////////////////////////////
 * //making sense of the Parallax PIR sensor's output
 * //////////////////////////////////////////////////
 *
 * Switches video recording according to the state of the sensors output pin.
 * Determines the beginning and end of continuous motion sequences.
 *
 * Original code is located at
 *   http://playground.arduino.cc/Code/PIRsense
 * @author: Kristian Gohlke / krigoo (_) gmail (_) com / http://krx.at
 * @date:   3. September 2006 
 * 
 * kr1 (cleft) 2006 
 * released under a creative commons "Attribution-NonCommercial-ShareAlike 2.0" license
 * http://creativecommons.org/licenses/by-nc-sa/2.0/de/
 *
 * Modified by orangkucing for MewPro (c) 2014
 *
 * The Parallax PIR Sensor is an easy to use digital infrared motion sensor module. 
 * ( http://www.parallax.com/product/555-28027 )
 *
 * The sensor's output pin goes to HIGH if motion is present.
 * However, even if motion is present it goes to LOW from time to time, 
 * which might give the impression no motion is present. 
 * This program deals with this issue by ignoring LOW-phases shorter than a given time, 
 * assuming continuous motion is present during these phases.
 *  
 */
#ifdef USE_PIR_SENSOR

void setupPIRSensor()
{
  pinMode(PIR_PIN, INPUT);
  __debug(F("PIR sensor calibration start (10 seconds)"));
}

void checkPIRSensor()
{
  //the time when the sensor outputs a low impulse
  static unsigned long lowIn;         

  //the amount of seconds the sensor has to be low 
  //before we assume all motion has stopped
  const unsigned long pause = 5;  

  static boolean lockLow = true;
  static boolean takeLowTime;

  if (millis() < 10000) { // Still calibrating...
    return;
  }
  if (digitalRead(PIR_PIN) == HIGH) {
    if (lockLow){  
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;            
      __debug(F("---"));
      __debug(F("motion detected at "));
#ifdef USE_TIME_ALARMS
      if (debug) {
        time_t t = now();
        char s[20];
        sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
        Serial_println(s);
      }
#else
      if (debug) {
        Serial_print(millis() / 1000);
      }
      __debug(F(" sec"));
#endif
      startRecording();
    }         
    takeLowTime = true;
  }

  if (digitalRead(PIR_PIN) == LOW) {
    if (takeLowTime) {
      lowIn = millis();          //save the time of the transition from high to LOW
      takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause, 
    //we assume that no more motion is going to happen
    if (!lockLow && millis() - lowIn > pause * 1000) {  
      //makes sure this block of code is only executed again after 
      //a new motion sequence has been detected
      lockLow = true;                        
      __debug(F("motion finished at "));      //output
#ifdef USE_TIME_ALARMS
      if (debug) {
        time_t t = now() - pause;
        char s[20];
        sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", year(t), month(t), day(t), hour(t), minute(t), second(t));
        Serial_println(s);
      }
#else
      if (debug) {
        Serial_print(millis() / 1000 - pause);
      }
      __debug(F(" sec"));
#endif
      stopRecording();
    }
  }
}

#else

void setupPIRSensor()
{
}

void checkPIRSensor()
{
}

#endif
