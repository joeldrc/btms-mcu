

volatile uint32_t timerValue = 1000;
volatile uint8_t cnt_cycle = 0;


FASTRUN void simulatedCycle1() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        digitalWriteFast(TEN, LOW); // enable internal timing

        digitalWriteFast(SSCY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, HIGH);

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 1: {
        digitalWriteFast(SECY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, HIGH);

        timerValue = ecyTime - scyTime; //time duration of the next cycle
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
      break;
  }
  simulatedTiming.update(timerValue);
}


FASTRUN void simulatedCycle2() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        digitalWriteFast(TEN, LOW); // enable internal timing

        digitalWriteFast(SSCY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, HIGH);

        timerValue = ecyTime - injTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 1: {
        digitalWriteFast(SINJ, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SINJ, HIGH);

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 2: {
        digitalWriteFast(SECY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, HIGH);

        timerValue = injTime - scyTime; //time duration of the next cycle
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
      break;
  }
  simulatedTiming.update(timerValue);
}


FASTRUN void simulatedCycle3() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        digitalWriteFast(TEN, LOW); // enable internal timing

        digitalWriteFast(SSCY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, HIGH);

        timerValue = calstopTime - calstartTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTART
    case 1: {
        digitalWriteFast(SCalStrt, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SCalStrt, HIGH);

        timerValue = injTime - calstopTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTOP
    case 2: {
        digitalWriteFast(SCalStp, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SCalStp, HIGH);

        timerValue = hchTime - injTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 3: {
        digitalWriteFast(SINJ, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SINJ, HIGH);

        timerValue = ecyTime - hchTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // HCH
    case 4: {
        digitalWriteFast(SHCH, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SHCH, HIGH);

        timerValue = psTimeCycle - ecyTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 5: {
        digitalWriteFast(SECY, LOW);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, HIGH);

        timerValue = calstartTime - scyTime; // time duration of the next cycle
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
  }
  simulatedTiming.update(timerValue);
}
