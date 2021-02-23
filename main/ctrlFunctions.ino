

void countCycle() {
  if (startOfcycle > 0) {
    Serial.println(elapsedMicros());
  }
}


FASTRUN void simulatedCycle() {
  digitalWriteFast(SSCY, HIGH);

  threads.delay(5);
  digitalWriteFast(SCalStrt, HIGH);
  threads.delay(5);
  digitalWriteFast(SCalStrt, LOW);

  threads.delay(100);
  digitalWriteFast(SCalStp, HIGH);
  threads.delay(5);
  digitalWriteFast(SCalStp, LOW);

  threads.delay(170);
  digitalWriteFast(SINJ, HIGH);
  threads.delay(5);
  digitalWriteFast(SINJ, LOW);

  digitalWriteFast(SHCH, LOW);
  digitalWriteFast(SECY, LOW);
}


FASTRUN void readCycle() {
  noInterrupts();
  traceTime[0] = startOfcycle;
  traceTime[1] = calStart;
  traceTime[2] = calStop;
  traceTime[3] = 0;
  traceTime[4] = harmonicChange;
  traceTime[5] = endOfCycle;
  interrupts();
  //digitalWriteFast(PIN, seq[ind]);
}
