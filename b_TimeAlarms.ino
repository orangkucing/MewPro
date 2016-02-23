// Timelapse photography driven by MewPro

#ifdef USE_TIME_ALARMS

boolean setupTimeAlarmsIsCalled = false; // to avoid setupTimeAlarms() to be called twice.

void AddMinutes(int *h, int *m, int x)
{
  *m += x;
  if (*m >= 60) {
    *h += *m / 60; *m %= 60;
    *h %= 24;
  }
}

void exampleAlarm()
{
  int h, m;
  queueIn(F("@")); // power on
  h = hour(); m = minute();
  AddMinutes(&h, &m, 1);
  Alarm.alarmOnce(h, m, 0, alarmStart);
}

void alarmStart()
{
  int h, m;
  startRecording();
  h = hour(); m = minute();
  AddMinutes(&h, &m, 1);
  Alarm.alarmOnce(h, m, 0, alarmStop);
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
    int h, m;
    setupTimeAlarmsIsCalled = true;
    __debug(F("Time alarms set"));
    // current time := hh:mm:xx
    // 1. power on hh:mm+1:30; start hh:mm+2:00; stop hh:mm+3:00; off hh:mm+3:05;
    // 2. power on hh:mm+3:30; start hh:mm+4:00; stop hh:mm+5:00; off hh:mm+5:05;
    // 3. power on hh:mm+5:30; start hh:mm+6:00; stop hh:mm+7:00; off hh:mm+7:05;
    // 4. power on hh:mm+7:30; start hh:mm+8:00; stop hh:mm+9:00; off hh:mm+9:05;   
    h = hour(); m = minute();
    AddMinutes(&h, &m, 1);
    Alarm.alarmRepeat(h, m, 30, exampleAlarm);
    AddMinutes(&h, &m, 2);
    Alarm.alarmRepeat(h, m, 30, exampleAlarm);
    AddMinutes(&h, &m, 2);
    Alarm.alarmRepeat(h, m, 30, exampleAlarm);
    AddMinutes(&h, &m, 2);
    Alarm.alarmRepeat(h, m, 30, exampleAlarm);
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
