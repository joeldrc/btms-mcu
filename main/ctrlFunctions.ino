

void countCycle() {
  if (startOfcycle > 0) {
    Serial.println(elapsedMicros());
  }
}


FASTRUN void simulateCycle() {
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
  //digitalWriteFast(PIN, seq[ind]);
}
