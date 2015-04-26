#define MEWPRO_BUFFER_LENGTH 64

byte queue[MEWPRO_BUFFER_LENGTH];
volatile int queueb = 0, queuee = 0;
volatile boolean waiting = false; // don't read the next command from the queue

void emptyQueue()
{
  queueb = queuee = 0;
  waiting = false;
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
  if (queueb != queuee) {
    byte c = queue[queueb];
    queueb = (queueb + 1) % MEWPRO_BUFFER_LENGTH;
    return c;
  }
  return Serial.read();
}

// Utility functions
void queueIn(const char *p)
{
  int i;
  for (i = 0; p[i] != 0; i++) {
    queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = p[i];
  }
  queue[(queuee + i) % MEWPRO_BUFFER_LENGTH] = '\n';
  queuee = (queuee + i + 1) % MEWPRO_BUFFER_LENGTH;
}
