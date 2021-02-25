/**
 ******************************************************************************
  @file    BTMS timing mcu
  @author  Joel Daricou  <joel.daricou@cern.ch>
  @brief   web server ctrl I/O
 ******************************************************************************
  Board:
    - Developped on teensy 4.1
    - CPU speed 600MHz
  Info programming:
    -
  References C++:
    -
  Scheme:
    -
 ******************************************************************************
*/

#include "defines.h"

#include <ADC.h>
#include <ADC_util.h>


/*** Global variables **/
// 0: read only
// 1: simulate SCY and ECY
// 2: simulate SCY, INJ and ECY
// 3: simulate SCY, CALSTATR, CALSTOP, INJ, HCH and ECY
uint8_t operationMode = 0;


// plot for webPage
const uint32_t samplesNumber = 150; // 2400m
const uint8_t numTraces = 5;
const char traceName[numTraces][10] = {{"SCY"}, {"CALSTRT"}, {"CALSTOP"}, {"INJ"}, {"ECY"}};

uint32_t traceTime[numTraces] = {0};


// other webPage
bool status1 = false;
bool status2 = false;
bool status3 = false;
bool status4 = false;
bool status5 = false;
float v1 = 0;
float v2 = 0;
float v3 = 0;
float v4 = 0;


// Timing object
elapsedMicros timing;


// Timer object
IntervalTimer simulatedTiming;


// Timer variables
const uint32_t pulseTime = 1;                  // time in uS
volatile const uint32_t psTimeCycle = 1200000; // time in uS

volatile const uint32_t scyTime = 0;           // time in uS
volatile const uint32_t calstartTime = 5000;   // time in uS
volatile const uint32_t calstopTime = 100000;  // time in uS
volatile const uint32_t injTime = 170000;      // time in uS
volatile const uint32_t hchTime = 400000;      // time in uS
volatile const uint32_t ecyTime = 805000;      // time in uS

volatile uint32_t timerValue = 10000;
volatile bool checkTiming = true;


// interrupt variables
volatile uint32_t startOfcycle = 0;
volatile uint32_t calStart = 0;
volatile uint32_t calStop = 0;
volatile uint32_t harmonicChange = 0;
volatile uint32_t endOfCycle = 0;


// interrupt functions
FASTRUN void interrupt_ISCY() {
  timing = 0;
  startOfcycle = timing;
}
FASTRUN void interrupt_ICalStrt() {
  calStart = timing;
}
FASTRUN void interrupt_ICalStp() {
  calStop = timing;
}
FASTRUN void interrupt_IHCH() {
  harmonicChange = timing;
}
FASTRUN void interrupt_IECY() {
  endOfCycle = timing;
}

FASTRUN void btn1() {
  // insert code
}
FASTRUN void btn2() {
  // insert code
}


uint8_t readSettingSwitch() {
  // Read the current four-bit
  uint8_t b = 0;
  // The switch are active LOW
  if (digitalReadFast(SW4) == 0) b |= 1;
  if (digitalReadFast(SW3) == 0) b |= 2;
  if (digitalReadFast(SW2) == 0) b |= 4;
  if (digitalReadFast(SW1) == 0) b |= 8;
  return (b);
}


void setup() {
  // Analog inputs
  pinMode(ADC10, INPUT);
  pinMode(ADC11, INPUT);
  pinMode(ADC12, INPUT);
  pinMode(ADC13, INPUT);

  // Buttons
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW6, INPUT_PULLUP);

  // Interrupts
  attachInterrupt(SW5, btn1, FALLING);
  //attachInterrupt(SW6, btn2, FALLING);

  // Outputs LED
  pinMode(StsLedOr, OUTPUT);
  pinMode(StsLedGr, OUTPUT);
  pinMode(StsLed1, OUTPUT);
  pinMode(StsLed2, OUTPUT);
  pinMode(StsLed3, OUTPUT);
  pinMode(StsLed4, OUTPUT);
  pinMode(StsLed5, OUTPUT);
  pinMode(StsLed6, OUTPUT);
  pinMode(StsLed7, OUTPUT);
  pinMode(StsLed8, OUTPUT);

  // Set LEDs off
  digitalWriteFast(StsLedOr, LOW);
  digitalWriteFast(StsLedGr, LOW);
  digitalWriteFast(StsLed1, LOW);
  digitalWriteFast(StsLed2, LOW);
  digitalWriteFast(StsLed3, LOW);
  digitalWriteFast(StsLed4, LOW);
  digitalWriteFast(StsLed5, LOW);
  digitalWriteFast(StsLed6, LOW);
  digitalWriteFast(StsLed7, LOW);
  digitalWriteFast(StsLed8, LOW);

  // Digital outputs
  pinMode(TEN, OUTPUT);
  pinMode(SSCY, OUTPUT);
  pinMode(SCalStrt, OUTPUT);
  pinMode(SCalStp, OUTPUT);
  pinMode(SINJ, OUTPUT);
  pinMode(SHCH, OUTPUT);
  pinMode(SECY, OUTPUT);

  digitalWriteFast(TEN, LOW);
  digitalWriteFast(SSCY, LOW);
  digitalWriteFast(SCalStrt, LOW);
  digitalWriteFast(SCalStp, LOW);
  digitalWriteFast(SINJ, LOW);
  digitalWriteFast(SHCH, LOW);
  digitalWriteFast(SECY, LOW);

  pinMode(BFrev1, OUTPUT);
  pinMode(BFrev2, OUTPUT);
  pinMode(BFrev3, OUTPUT);
  pinMode(BFrev4, OUTPUT);

  digitalWriteFast(BFrev1, LOW);
  digitalWriteFast(BFrev2, LOW);
  digitalWriteFast(BFrev3, LOW);
  digitalWriteFast(BFrev4, LOW);

  pinMode(FastGPIO1, OUTPUT);
  pinMode(FastGPIO2, OUTPUT);
  pinMode(FastGPIO3, OUTPUT);

  digitalWriteFast(FastGPIO1, LOW);
  digitalWriteFast(FastGPIO2, LOW);
  digitalWriteFast(FastGPIO3, LOW);

  // Digital inputs
  pinMode(_10MHzDet, INPUT);
  pinMode(D10MHz, INPUT);
  pinMode(Lock, INPUT);

  pinMode(ISCY, INPUT);
  pinMode(ICalStrt, INPUT);
  pinMode(ICalStp, INPUT);
  pinMode(IHCH, INPUT);
  pinMode(IECY, INPUT);

  // Interrupts
  attachInterrupt(ISCY, interrupt_ISCY, RISING);
  attachInterrupt(ICalStrt, interrupt_ICalStrt, RISING);
  attachInterrupt(ICalStp, interrupt_ICalStp, RISING);
  attachInterrupt(IHCH, interrupt_IHCH, RISING);
  attachInterrupt(IECY, interrupt_IECY, RISING);

  // Start serial
  Serial.begin(9600);
  Serial.println("BTMS mcu serial monitor");

  // Timer setup
  // Set the interrupt priority level,
  // controlling which other interrupts this timer is allowed to interrupt.
  // Lower numbers are higher priority, with 0 the highest and 255 the lowest.
  simulatedTiming.priority(1);
}


void loop() {
  static int previousSetting = -1;

  // fast cycles
  if (previousSetting != operationMode) {
    switch (operationMode) {
      case 0: {
          simulatedTiming.begin(readOnly, psTimeCycle);
          digitalWriteFast(StsLedOr, LOW);
        }
        break;

      case 1: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle1, psTimeCycle);
          digitalWriteFast(StsLedOr, HIGH);
        }
        break;

      case 2: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle2, psTimeCycle);
          digitalWriteFast(StsLedOr, HIGH);
        }
        break;

      case 3: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle3, psTimeCycle);
          digitalWriteFast(StsLedOr, HIGH);
        }
        break;
      default: {
          operationMode = 0;
        }
        break;
    }
    Serial.print("Operation mode: ");
    Serial.println(operationMode);
    previousSetting = operationMode;
  }


  // read incoming timing on the interrupts
  readCycle();


  // slow cycle
  if (checkTiming) {
    ctrlLoop();

    noInterrupts();
    checkTiming = false;
    interrupts();
  }

  webServer_thread();
}
