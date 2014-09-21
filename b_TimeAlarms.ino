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

#include <Time.h>
#include <TimeAlarms.h>

boolean setupTimeAlarmsIsCalled = false; // to avoid setupTimeAlarms() to be called twice.

void alarmPowerOn()
{
  queueIn("@"); // power on
}

void alarmShutter()
{
  startRecording();
}

void alarmSuspend()
{
  queueIn("PW0"); // SET_CAMERA_POWER_STATE off
}

void setupTimeAlarms()
{
  if (!setupTimeAlarmsIsCalled) {
    setupTimeAlarmsIsCalled = true;
    Serial.println("Time alarms set");
    // Example of timelapse
    //   sixty-minutes intervals on the hour.
/*
    for (int hour = 0; hour < 24; hour++) {
      Alarm.alarmRepeat(hour, 59, 50, alarmPowerOn);
      Alarm.alarmRepeat(hour, 0, 0, alarmShutter);
      Alarm.alarmRepeat(hour, 0, 10, alarmSuspend);
    }
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
