// Timelapse photography driven by MewPro
// Time and TimeAlarms are downloadable from
//   http://www.pjrc.com/teensy/td_libs_Time.html
//   and http://www.pjrc.com/teensy/td_libs_TimeAlarms.html
// In order to compile the code on Pro Mini 328, find the following and edit the first line of them
//   #if defined(__AVR__)
//   #include <avr/pgmspace.h>
//   #else
// to
//   #if defined(__AVR__) && !defined(__AVR_ATmega328P__)
//   #include <avr/pgmspace.h>
//   #else
// appeared in Documents/Arduino/libraries/Time/DateStrings.cpp

#ifdef USE_TIME_ALARMS

//#include <Time.h> // *** please comment out this line if USE_TIME_ALARMS is not defined ***
//#include <TimeAlarms.h> // *** please comment out this line if USE_TIME_ALARMS is not defined ***

boolean setupTimeAlarmsIsCalled = false; // to avoid setupTimeAlarms() to be called twice.

time_t alarmtime;

void alarmPowerOn()
{
  alarmtime = now();
  queueIn("@"); // power on
  Alarm.alarmOnce((hour(alarmtime)+1) % 24, 0, 0, alarmShutter);
}

void alarmShutter()
{
  startRecording();
  Alarm.alarmOnce((hour(starttime)+1) % 24, 0, 10, alarmSuspend);
}

void alarmSuspend()
{
  alarmtime = now();
  queueIn("PW0"); // SET_CAMERA_POWER_STATE off
  if (minute(alarmtime) == 59) {
    Alarm.alarmOnce((hour(alarmtime)+1) % 24, 59, 50, alarmPowerOn);
  } else {
    Alarm.alarmOnce(hour(alarmtime), 59, 50, alarmPowerOn);
  }
}

void setupTimeAlarms()
{  
  if (!setupTimeAlarmsIsCalled) {
    setupTimeAlarmsIsCalled = true;
    Serial.println(F("Time alarms set"));
    // Example of timelapse
    //   sixty-minutes intervals on the hour.
/*
    alarmSuspend();
 */
  }
}

void checkTimeAlarms()
{
  Alarm.delay(0); // Don't delete. Alarms are serviced from here.
}

#else

void setupTimeAlarms()
{
}

void checkTimeAlarms()
{
}

#endif
