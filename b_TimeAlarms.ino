// Time alarm video/photo driven by MewPro

#ifdef USE_TIME_ALARMS

#define delay(a) Alarm.delay(a)

volatile boolean alarmEntered = false;   // in order to avoid timeAdjust() or alarmPowerOn() functions called twice
const int syncDelay = 600;               // more than 8 minute. workaround for time drift on Arduino
const int powerOnDelay = 60;             // delay in second after camera power on 
const int powerOffDelay = 5;             // delay in second before camera power off

// Since the time measured by Arduino's Time library gains or loses maximum 20 seconds per hour, we need to sync the time with GoPro's RTC at least once a day.
void timeAdjust()
{
  if (!alarmEntered) {
    alarmEntered = true;
    queueIn(F("@"));
    Alarm.timerOnce(powerOnDelay, alarmSuspend);
  }
}

// Alarm tasks
void alarmPowerOn()
{
  if (!alarmEntered) {
    alarmEntered = true;
    queueIn(F("@"));
    Alarm.timerOnce(powerOnDelay, alarmStartRecording);
  }
}

void alarmStartRecording()
{
  startRecording();
/***********************
Example alarm: Shooting duration in seconds
 *** EDIT NEXT LINE ****/
  Alarm.timerOnce(5 * 60, alarmStopRecording);  // 5 minutes
/***********************/
}

void alarmStopRecording()
{
  stopRecording();
  Alarm.timerOnce(5, alarmSuspend);  // power off at powerOffDelay seconds after stop
}

void alarmSuspend()
{
  queueIn(F("PW0"));
  // alarmEntered == true for a long enough time > 8 minute (> 20 * 24 seconds).
  Alarm.timerOnce(syncDelay, alarmFlagClear);
}

void alarmFlagClear()
{
  alarmEntered = false;
}

void _setTime() // This function is called after every time when the camera power on
{
  TimeElements tm;
  tm.Hour = td[TD_DATE_TIME_hour]; tm.Minute = td[TD_DATE_TIME_minute]; tm.Second = td[TD_DATE_TIME_second];
  tm.Day = td[TD_DATE_TIME_day]; tm.Month = td[TD_DATE_TIME_month]; tm.Year = y2kYearToTm(td[TD_DATE_TIME_year]);
  setTime(makeTime(tm));
  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    // delete all alarms
    if (Alarm.isAlarm(id)) {
      Alarm.free(id);
    }
  }

/***********************
Example alarm: Shooting video from 09:00
 *** EDIT NEXT LINE ****/
  tm.Hour = 9; tm.Minute = 0; tm.Second = 0; // 09:00:00  Recording start time
/***********************/

  time_t t = makeTime(tm), s;
  // Arduino's time is adjusted whenever GoPro power on.
  // Sync the time once a day. Take care the time before power-on has drifted a maximum of 8 minute!
  s = t - syncDelay - powerOnDelay * 2 - 60;
  Alarm.alarmRepeat(hour(s), minute(s), second(s), timeAdjust);
  //
  // just after sync with GoPro, the time is accurate enough. let's power on again and shoot video.
  t -= powerOnDelay; // powerOnDelay seconds before recording start
/***********************
 *** EDIT NEXT AND CHOOSE EITHER ALARM 1 or 2 or 3 ****/
/***********************
Example alarm 1: Shooting video everyday */
  Alarm.alarmRepeat(hour(t), minute(t), second(t), alarmPowerOn);
/***********************
Example alarm 2: Shooting video weekly on Tuesday */
//  Alarm.alarmRepeat(dowTuesday, hour(t), minute(t), second(t), alarmPowerOn);
/***********************
Example alarm 3: Shooting video on Monday and Wednesday and Friday */
//  Alarm.alarmRepeat(dowMonday, hour(t), minute(t), second(t), alarmPowerOn);
//  Alarm.alarmRepeat(dowWednesday, hour(t), minute(t), second(t), alarmPowerOn);
//  Alarm.alarmRepeat(dowFriday, hour(t), minute(t), second(t), alarmPowerOn);
/***********************/
}

void checkTimeAlarms()
{
  delay(0); // Don't delete. Alarms are serviced from here.
}

#else

void _setTime()
{
}

void checkTimeAlarms()
{
}

#endif
