

volatile uint32_t timerValue = 0;
volatile uint8_t cnt_cycle = 0;


FASTRUN void triggerSCY() {
  digitalWriteFast(SSCY, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SSCY, HIGH);
}

FASTRUN void triggerCalStrt() {
  digitalWriteFast(SCalStrt, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SCalStrt, HIGH);
}

FASTRUN void triggerCalStp() {
  digitalWriteFast(SCalStp, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SCalStp, HIGH);
}

FASTRUN void triggerINJ() {
  digitalWriteFast(SINJ, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SINJ, HIGH);
}

FASTRUN void triggerHCH() {
  digitalWriteFast(SHCH, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SHCH, HIGH);
}

FASTRUN void triggerECY() {
  digitalWriteFast(SECY, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SECY, HIGH);
}


FASTRUN void simulatedCycle1() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        digitalWriteFast(TEN, LOW); // enable internal timing
        triggerSCY();

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 1: {
        triggerECY();

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
        triggerSCY();

        timerValue = ecyTime - injTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 1: {
        triggerINJ();

        timerValue = psTimeCycle - ecyTime; //time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 2: {
        triggerECY();

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
        triggerSCY();

        timerValue = calstopTime - calstartTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTART
    case 1: {
        triggerCalStrt();

        timerValue = injTime - calstopTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // CALSTOP
    case 2: {
        triggerCalStp();

        timerValue = hchTime - injTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // INJ
    case 3: {
        triggerINJ();

        timerValue = ecyTime - hchTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // HCH
    case 4: {
        triggerHCH();

        timerValue = psTimeCycle - ecyTime; // time duration of the next cycle
        cnt_cycle++;
      }
      break;
    // ECY
    case 5: {
        triggerECY();

        timerValue = calstartTime - scyTime; // time duration of the next cycle
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
  }
  simulatedTiming.update(timerValue);
}
