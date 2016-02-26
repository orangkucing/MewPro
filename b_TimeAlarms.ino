// Timelapse photography driven by MewPro

#ifdef USE_TIME_ALARMS

boolean setupTimeAlarmsIsCalled = false; // to avoid setupTimeAlarms() to be called twice.

void exampleAlarm()
{
  queueIn(F("@"));
  Alarm.timerOnce(30, alarmStart); // 30 seconds delay for start up
}

void alarmStart()
{
  startRecording();
  Alarm.timerOnce(60, alarmStop); // take video for 1 minutes
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
    time_t t;
    setupTimeAlarmsIsCalled = true;
    __debug(F("Time alarms set"));
    // X = current time
    //   1. power on X + 60s; start X + 90s; stop X + 150s; off X + 155s;
    //   2. power on X + 180s; start X + 210s; stop X + 270s; off X + 275s;
    // and repeat 1, 2 everyday.
    t = now() + 60;
    Alarm.alarmRepeat(hour(t), minute(t), second(t), exampleAlarm);
    t += 120;
    Alarm.alarmRepeat(hour(t), minute(t), second(t), exampleAlarm);
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
