// Timelapse photography driven by MewPro

#ifdef USE_TIME_ALARMS

boolean setupTimeAlarmsIsCalled = false; // to avoid setupTimeAlarms() to be called twice.

void WeeklyAlarm()
{
  queueIn(F("@")); // power on
  Alarm.alarmOnce(9, 0, 0, alarmStart); // start time 09:00:00
}

void alarmStart()
{
  startRecording();
  Alarm.alarmOnce(9, 5, 0, alarmStop); // stop time 09:05:00
}

void alarmStop()
{
  stopRecording();
  Alarm.timerOnce(5, alarmSuspend); // power off at 5 seconds after stop
}

void alarmSuspend()
{
  queueIn(F("PW0"));
}

void _setTime() {
  setTime(td[TD_DATE_TIME_hour], td[TD_DATE_TIME_minute], td[TD_DATE_TIME_second], td[TD_DATE_TIME_day], td[TD_DATE_TIME_month], 2000+td[TD_DATE_TIME_year]);
}

void setupTimeAlarms()
{
  if (!setupTimeAlarmsIsCalled) {
    setupTimeAlarmsIsCalled = true;
    __debug(F("Time alarms set"));
    // this sample code will start the camera at a certain time of day, for a set period, then turn off for a period of seven days.
    // start video record, repeat every Saturday start time 09:00, finish time 09:05, power off
    Alarm.alarmRepeat(dowSaturday, 8, 59, 45, WeeklyAlarm); // power on at 15 seconds before start
    // suspend for now
    //alarmSuspend();
  }
}

void checkTimeAlarms()
{
  Alarm.delay(0); // Don't delete. Alarms are serviced from here.
}

#else

void _setTime() {
}

void setupTimeAlarms()
{
}

void checkTimeAlarms()
{
}

#endif
