// Interface to CANON Timer Remote Controller TC-80N3
//   Also works with similar remote shutters.
//
// Note: For simple mechanical switches, please use USE_SWITCHES instead
// (Simple switches make chatters so software debouncing is necessary).
#ifdef USE_SHUTTERS

// Workaround the bug in attachInterrupt():
//   Calling attachInterrupt() first time causes the interrupt handler
//   to be called once. Maybe pending/nonprocessed interrupts get alive again
//   and fire the interrupt routine.
boolean pendingInterrupts = true;

void shutterHandler()
{
  if (!pendingInterrupts) {
    if (digitalRead(SHUTTER_PIN) == LOW) {
      startRecording();
    } else {
      stopRecording();
    }
  }
}

void setupShutter()
{
  pinMode(SHUTTER_PIN, INPUT_PULLUP);
  attachInterrupt(SHUTTER_PIN + INT_NUMBER_OFFSET, shutterHandler, CHANGE); 
  pendingInterrupts = false;
}


#else

void setupShutter()
{
}

#endif
