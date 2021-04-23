

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


FASTRUN void simulatedCycle() {
  switch (cnt_cycle) {
    // SCY
    case 0: {
        simulatedTiming.begin(simulatedCycle, calstartTime); // time before the next cycle
        triggerSCY();

        // Status leds
        digitalWriteFast(StsLed1, LOW);
        digitalWriteFast(StsLed2, LOW);
        digitalWriteFast(StsLed3, LOW);
        digitalWriteFast(StsLed4, LOW);
        cnt_cycle++;
      }
      break;
    // CALSTART
    case 1: {
        simulatedTiming.begin(simulatedCycle, calstopTime - calstartTime); // time before the next cycle
        if (calStartSimulation) {
          triggerCalStrt();
        }
        cnt_cycle++;
      }
      break;
    // CALSTOP
    case 2: {
        simulatedTiming.begin(simulatedCycle, injTime - calstopTime); // time before the next cycle
        if (calStopSimulation) {
          triggerCalStp();
        }
        cnt_cycle++;
      }
      break;
    // INJ
    case 3: {
        simulatedTiming.begin(simulatedCycle, hchTime - injTime); // time before the next cycle
        if (injSimulation) {
          triggerINJ();
        }
        cnt_cycle++;
      }
      break;
    // HCH
    case 4: {
        simulatedTiming.begin(simulatedCycle, ecyTime - hchTime); // time before the next cycle
        if (hchSimulation) {
          triggerHCH();
        }
        cnt_cycle++;
      }
      break;
    // ECY
    case 5: {
        simulatedTiming.begin(simulatedCycle, psTimeCycle - ecyTime); // time before the next cycle
        triggerECY();

        // Status leds
        digitalWriteFast(StsLed1, HIGH);
        digitalWriteFast(StsLed2, HIGH);
        digitalWriteFast(StsLed3, HIGH);
        digitalWriteFast(StsLed4, HIGH);
      }
    // PSB cycle 1.2 sec
    default: {
        cnt_cycle = 0;
      }
  }
}
