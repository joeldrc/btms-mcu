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


void ctrlLedThread() {
  while (1) {
    static bool ledVal = false;
    digitalWrite(StsLed1, ledVal);
    ledVal = !ledVal;
    threads.delay(100);
    threads.yield();
  }
}


void setup() {
  // set the digital pin as input
  pinMode(IECY, INPUT);
  pinMode(ICalStp, INPUT);
  pinMode(ICalStrt, INPUT);
  pinMode(ISCY, INPUT);
  pinMode(Lock, INPUT);

  pinMode(ADC10, INPUT);
  pinMode(ADC11, INPUT);
  pinMode(ADC12, INPUT);
  pinMode(ADC13, INPUT);

  pinMode(_10MHzDet, INPUT);
  pinMode(D10MHz, INPUT);
  pinMode(IHCH, INPUT);

  pinMode(SW6, INPUT_PULLUP);
  pinMode(SW5, INPUT_PULLUP);
  pinMode(SW4, INPUT_PULLUP);
  pinMode(SW3, INPUT_PULLUP);
  pinMode(SW2, INPUT_PULLUP);
  pinMode(SW1, INPUT_PULLUP);
  pinMode(SECY, OUTPUT);
  pinMode(SSCY, OUTPUT);
  pinMode(SHCH, OUTPUT);
  pinMode(BFrev4, OUTPUT);
  pinMode(TEN, OUTPUT);
  pinMode(FastGPIO1, OUTPUT);
  pinMode(SINJ, OUTPUT);
  pinMode(BFrev3, OUTPUT);
  pinMode(StsLed1, OUTPUT);
  pinMode(FastGPIO3, OUTPUT);
  pinMode(FastGPIO2, OUTPUT);
  pinMode(StsLedOr, OUTPUT);
  pinMode(StsLedGr, OUTPUT);
  pinMode(SCalStp, OUTPUT);
  pinMode(BFrev2, OUTPUT);
  pinMode(SCalStrt, OUTPUT);
  pinMode(BFrev1, OUTPUT);
  pinMode(StsLed8, OUTPUT);
  pinMode(StsLed7, OUTPUT);
  pinMode(StsLed6, OUTPUT);
  pinMode(StsLed5, OUTPUT);
  pinMode(StsLed4, OUTPUT);
  pinMode(StsLed3, OUTPUT);
  pinMode(StsLed2, OUTPUT);


  Serial.begin(9600);
  Serial.println("BTMS mcu serial monitor");

  threads.addThread(ctrlLedThread, 1);
  threads.addThread(ctrlConnection, 1);
  threads.addThread(ethernetConfig_thread, 1);
}


void loop() {
  webServer_thread();

  digitalReadFast(_10MHzDet);
}
