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
#include "src\TeensyThreads\TeensyThreads.h"


// Global variables
bool status1 = false;
bool status2 = false;
bool status3 = false;
bool status4 = false;
bool status5 = false;

float v1 = 0;
float v2 = 0;
float v3 = 0;
float v4 = 0;

// Interrupt variables
volatile uint32_t startOfcycle = 0;
volatile uint32_t calStart = 0;
volatile uint32_t calStop = 0;
volatile uint32_t harmonicChange = 0;
volatile uint32_t endOfCycle = 0;


FASTRUN void interrupt_ISCY() {
  startOfcycle = millis();
}
FASTRUN void interrupt_ICalStrt() {
  calStart = millis();
}
FASTRUN void interrupt_ICalStp() {
  calStop = millis();
}
FASTRUN void interrupt_IHCH() {
  harmonicChange = millis();
}
FASTRUN void interrupt_IECY() {
  endOfCycle = millis();
}


void ctrlLedThread() {
  while (1) {
    static bool ledVal = false;
    digitalWrite(StsLed1, ledVal);
    ledVal = !ledVal;

    Serial.println(startOfcycle);
    Serial.println(calStart);
    Serial.println(calStop);
    Serial.println(harmonicChange);
    Serial.println(endOfCycle);

    threads.delay(100);
    threads.yield();
  }
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

  // Start threads
  threads.addThread(ctrlLedThread, 1);
  threads.addThread(ctrlConnection, 1);
  threads.addThread(ethernetConfig_thread, 1);
}


void loop() {
  webServer_thread();
  simulateCycle();
  readCycle();
}
