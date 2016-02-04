#define MEWPRO_BUFFER_LENGTH 64

byte queue[MEWPRO_BUFFER_LENGTH];
volatile int queueb = 0, queuee = 0;
boolean waiting = false; // don't read the next command from the queue
boolean serialfirst = false;

void emptyQueue()
{
  queueb = queuee = 0;
  waiting = false;
  serialfirst = false;
}

boolean inputAvailable()
{
  if (!waiting && (queueb != queuee || Serial.available())) {
    return true;
  }
  return false;
}

byte myRead()
{
  if (serialfirst && Serial.available()) {
    return Serial.read();
  }
  if (queueb != queuee) {
    byte c = queue[queueb];
    queueb = (queueb + 1) % MEWPRO_BUFFER_LENGTH;
    serialfirst = false;
    return c;
  }
  serialfirst = true;
  return Serial.read();
}

// Utility functions
void queueIn(const __FlashStringHelper *p)
{
  int i;
  char c;
  for (i = 0; (c = pgm_read_byte((char PROGMEM *)p + i)) != 0; i++) {
    queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = c;
  }
  queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = '\n';
  queuee = (queuee + i + 1) % MEWPRO_BUFFER_LENGTH;
}

void queueIn(const char *p)
{
  int i;
  char c;
  for (i = 0; (c = *(p + i)) != 0; i++) {
    queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = c;
  }
  queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = '\n';
  queuee = (queuee + i + 1) % MEWPRO_BUFFER_LENGTH;
}
