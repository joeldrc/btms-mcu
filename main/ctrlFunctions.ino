

volatile uint32_t timerValue = 10000;
volatile uint8_t cnt_cycle = 0;


FASTRUN void simulatedCycle1() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        digitalWriteFast(TEN, HIGH);

        digitalWriteFast(SSCY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 1: {
        digitalWriteFast(SECY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, LOW);

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
        digitalWriteFast(TEN, HIGH);

        digitalWriteFast(SSCY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);

        timerValue = ecyTime - injTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 1: {
        digitalWriteFast(SINJ, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SINJ, LOW);

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 2: {
        digitalWriteFast(SECY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, LOW);

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
        digitalWriteFast(TEN, HIGH);

        digitalWriteFast(SSCY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);

        timerValue = calstopTime - calstartTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTART
    case 1: {
        digitalWriteFast(SCalStrt, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SCalStrt, LOW);

        timerValue = injTime - calstopTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTOP
    case 2: {
        digitalWriteFast(SCalStp, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SCalStp, LOW);

        timerValue = hchTime - injTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 3: {
        digitalWriteFast(SINJ, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SINJ, LOW);

        timerValue = ecyTime - hchTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // HCH
    case 4: {
        digitalWriteFast(SHCH, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SHCH, LOW);

        timerValue = psTimeCycle - ecyTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 5: {
        digitalWriteFast(SECY, HIGH);
        delayMicroseconds(pulseTime);
        digitalWriteFast(SECY, LOW);

        timerValue = calstartTime - scyTime; // time duration of the next cycle
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
  }
  simulatedTiming.update(timerValue);
}
