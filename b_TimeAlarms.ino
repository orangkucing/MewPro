// Timelapse photography driven by MewPro

#ifdef USE_TIME_ALARMS

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
