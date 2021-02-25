

volatile uint8_t cnt_cycle = 0;


FASTRUN void readOnly() {
  checkTiming = true;
  cnt_cycle = 0;
}


FASTRUN void simulatedCycle1() {
  switch (cnt_cycle) {
    // ECY
    case 1: {
        simulatedTiming.update(psTimeCycle - ecyTime);
        digitalWriteFast(SECY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SECY, LOW);
      }
      break;
    // PSB cycle 1.2 sec
    default: {
        checkTiming = true;
        cnt_cycle = 0;
      }
    // SCY
    case 0: {
        simulatedTiming.update(ecyTime - scyTime);
        digitalWriteFast(SSCY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);
      }
      break;
  }
  cnt_cycle++;
}

FASTRUN void simulatedCycle2() {
  switch (cnt_cycle) {
    // INJ
    case 1: {
        simulatedTiming.update(ecyTime - injTime);
        digitalWriteFast(SINJ, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SINJ, LOW);
      }
      break;
    // ECY
    case 2: {
        simulatedTiming.update(psTimeCycle - ecyTime);
        digitalWriteFast(SECY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SECY, LOW);
      }
      break;
    // PSB cycle 1.2 sec
    default: {
        checkTiming = true;
        cnt_cycle = 0;
      }
    // SCY
    case 0: {
        simulatedTiming.update(injTime - scyTime);
        digitalWriteFast(SSCY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);
      }
      break;
  }
  cnt_cycle++;
}


FASTRUN void simulatedCycle3() {
  switch (cnt_cycle) {
    // CALSTART
    case 1: {
        simulatedTiming.update(calstopTime - calstartTime);
        digitalWriteFast(SCalStrt, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SCalStrt, LOW);
      }
      break;
    // CALSTOP
    case 2: {
        simulatedTiming.update(injTime - calstopTime);
        digitalWriteFast(SCalStp, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SCalStp, LOW);
      }
      break;
    // INJ
    case 3: {
        simulatedTiming.update(hchTime - injTime);
        digitalWriteFast(SINJ, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SINJ, LOW);
      }
      break;
    // HCH
    case 4: {
        simulatedTiming.update(ecyTime - hchTime);
        digitalWriteFast(SHCH, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SHCH, LOW);
      }
      break;
    // ECY
    case 5: {
        simulatedTiming.update(psTimeCycle - ecyTime);
        digitalWriteFast(SECY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SECY, LOW);
      }
      break;
    // PSB cycle 1.2 sec
    default: {
        checkTiming = true;
        cnt_cycle = 0;
      }
    // SCY
    case 0: {
        simulatedTiming.update(calstartTime - scyTime);
        digitalWriteFast(SSCY, HIGH);
        delayNanoseconds(pulseTime);
        digitalWriteFast(SSCY, LOW);
      }
      break;
  }
  cnt_cycle++;
}


FASTRUN void readCycle() {
  noInterrupts();
  traceTime[0] = startOfcycle;
  traceTime[1] = calStart;
  traceTime[2] = calStop;
  traceTime[3] = harmonicChange;
  traceTime[4] = endOfCycle;
  interrupts();
}


void ctrlLoop() {
  static bool ledVal = false;
  digitalWriteFast(StsLed1, ledVal);
  digitalWriteFast(StsLedGr, ledVal);
  ledVal = !ledVal;

  if (ctrlConnection() != true) {
    ethernetConfig_thread();
  }
  Serial.print("Setting switch: ");
  Serial.print(readSettingSwitch());
  Serial.println();
}
