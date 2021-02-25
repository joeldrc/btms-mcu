

volatile uint8_t cnt_cycle = 0;

FASTRUN void simulatedCycle() {
  switch (cnt_cycle) {

    // CALSTART
    case 1: {
        simulatedTiming.update(calstartTime - calstopTime);
        digitalWriteFast(SCalStrt, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SCalStrt, LOW);
      }
      break;

    // CALSTOP
    case 2: {
        simulatedTiming.update(calstopTime - injTime);
        digitalWriteFast(SCalStp, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SCalStp, LOW);
      }
      break;

    // INJ
    case 3: {
        simulatedTiming.update(injTime - hchTime);
        digitalWriteFast(SINJ, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SINJ, LOW);
      }
      break;

    // HCH
    case 4: {
        simulatedTiming.update(hchTime - ecyTime);
        digitalWriteFast(SHCH, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SHCH, LOW);
      }
      break;

    // ECY
    case 5: {
        simulatedTiming.update(ecyTime - psTimeCycle);
        digitalWriteFast(SECY, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SECY, LOW);
      }
      break;

    // PSB cycle 1.2 sec
    case 6: {
        checkTiming = true;
        cnt_cycle = 0;
      }

    // SCY
    case 0: {
        simulatedTiming.update(scyTime - calstartTime);
        digitalWriteFast(SSCY, HIGH);
        delayNanoseconds(100);
        digitalWriteFast(SSCY, LOW);
      }
      break;
  }

  cnt_cycle++;
  //__asm__ volatile("nop" "\n\t");
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
}


void ctrlLoop() {
  static bool ledVal = false;
  digitalWriteFast(StsLed1, ledVal);
  ledVal = !ledVal;

  if (ctrlConnection() != true) {
    ethernetConfig_thread();
  }
  Serial.print("Setting switch: ");
  Serial.print(readSettingSwitch());
  Serial.println();
}


void ctrlLedThread() {
  static bool ledVal = false;
  digitalWriteFast(StsLedOr, ledVal);
  digitalWriteFast(StsLedGr, !ledVal);
  ledVal = !ledVal;
}
