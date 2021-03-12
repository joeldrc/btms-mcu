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

#include <Bounce.h>
#include "src\TeensyThreads\TeensyThreads.h"


Bounce pushbutton1 = Bounce(SW6, 10);  // 10 ms debounce
Bounce pushbutton2 = Bounce(SW5, 10);  // 10 ms debounce


// chip temperature
extern float tempmonGetTemp(void);


/*** Global variables **/
// 0: read only
// 1: simulate SCY and ECY
// 2: simulate SCY, INJ and ECY
// 3: simulate SCY, CALSTATR, CALSTOP, INJ, HCH and ECY
volatile uint32_t operationMode = 0;
volatile bool boardStatus = 0;
volatile bool det10Mhz = 0;
volatile bool lock = 0;

volatile float cpuTemp = 0;


// plot for webPage
const uint32_t samplesNumber = 240; // 1200 milliseconds / 5 = 240 samples
const uint8_t numTraces = 5;
const char traceName[numTraces][10] = {{"SCY"}, {"CALSTRT"}, {"CALSTOP"}, {"INJ"}, {"ECY"}};
uint32_t traceTime[numTraces] = {0};
bool plot[numTraces][samplesNumber] = {0};


// other webPage
float v1 = 0;
float v2 = 0;
float v3 = 0;
float v4 = 0;


// Timing object
elapsedMicros timing;


// Timer object
IntervalTimer simulatedTiming;


// Timer variables
const uint32_t pulseTime = 1;                 // time in uS

// Timing for each cycle
volatile uint32_t scyTime = SCY_T;            // time in uS
volatile uint32_t calstartTime = CALSTART_T;  // time in uS
volatile uint32_t calstopTime = CALSTOP_T;    // time in uS
volatile uint32_t injTime = INJ_T;            // time in uS
volatile uint32_t hchTime = HCH_T;            // time in uS
volatile uint32_t ecyTime = ECY_T;            // time in uS
volatile uint32_t psTimeCycle = PSCYCLE_T;    // time in uS


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


void ctrlEthernetThread() {
  while (1) {
    ctrlConnection();
    ethernetConfig_thread();

    threads.delay(4000);
    threads.yield();
  }
}


void displayLeds(int byt) {
  int d = 0;
  int binary[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  if (byt <= 255) {
    while (byt > 0) {
      binary[d] = byt % 2;
      byt = byt / 2;
      d++;
    }
    if (binary[0] == 1) digitalWrite(StsLed1, HIGH);
    else digitalWrite(StsLed1, LOW);
    if (binary[1] == 1) digitalWrite(StsLed2, HIGH);
    else digitalWrite(StsLed2, LOW);
    if (binary[2] == 1) digitalWrite(StsLed3, HIGH);
    else digitalWrite(StsLed3, LOW);
    if (binary[3] == 1) digitalWrite(StsLed4, HIGH);
    else digitalWrite(StsLed4, LOW);
    if (binary[4] == 1) digitalWrite(StsLed5, HIGH);
    else digitalWrite(StsLed5, LOW);
    if (binary[5] == 1) digitalWrite(StsLed6, HIGH);
    else digitalWrite(StsLed6, LOW);
    if (binary[6] == 1) digitalWrite(StsLed7, HIGH);
    else digitalWrite(StsLed7, LOW);
    if (binary[7] == 1) digitalWrite(StsLed8, HIGH);
    else digitalWrite(StsLed8, LOW);
  }
}


void heartBeatThread() {
  while (1) {
    // Set status leds
    static bool ledVal = false;
    digitalWrite(StsLedGr, ledVal);
    digitalWrite(StsLedOr, HIGH);
    ledVal = !ledVal;

    // Set front panel leds
    static uint8_t val = 1;
    switch (operationMode) {
      case 0: {
          if (val < 128) {
            val = val << 1;
          }
          else {
            val = 1;
          }
        }
        break;

      case 1: {
          val = 0b00000011;
        }
        break;

      case 2: {
          val = 0b00000111;
        }
        break;

      case 3: {
          val = 0b00001111;
        }
        break;
    }
    displayLeds(val);

    // Read hardware switch
    Serial.print("Setting switch: ");
    Serial.print(readSettingSwitch());
    Serial.println();

    // Read digital inputs
    det10Mhz = digitalReadFast(_10MHzDet);
    lock = digitalReadFast(Lock);

    // Read analog values
    v1 = analogRead(ADC10);
    v2 = analogRead(ADC11);
    v3 = analogRead(ADC12);
    v4 = analogRead(ADC13);

    v1 = map(v1, 0, 4095, 0, 50);
    v2 = map(v2, 0, 4095, -21.6, 0);
    v3 = map(v3, 0, 4095, 0, 18.3);
    v4 = map(v4, 0, 4095, 0, 8.3);

    // read CPU temperature
    cpuTemp = tempmonGetTemp();

    threads.delay(500);
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
  digitalWrite(StsLedOr, LOW);
  digitalWrite(StsLedGr, LOW);
  digitalWrite(StsLed1, LOW);
  digitalWrite(StsLed2, LOW);
  digitalWrite(StsLed3, LOW);
  digitalWrite(StsLed4, LOW);
  digitalWrite(StsLed5, LOW);
  digitalWrite(StsLed6, LOW);
  digitalWrite(StsLed7, LOW);
  digitalWrite(StsLed8, LOW);

  // Digital outputs
  pinMode(TEN, OUTPUT);
  pinMode(SSCY, OUTPUT);
  pinMode(SCalStrt, OUTPUT);
  pinMode(SCalStp, OUTPUT);
  pinMode(SINJ, OUTPUT);
  pinMode(SHCH, OUTPUT);
  pinMode(SECY, OUTPUT);

  digitalWriteFast(TEN, LOW);

  // active low signals
  digitalWriteFast(SSCY, HIGH);
  digitalWriteFast(SCalStrt, HIGH);
  digitalWriteFast(SCalStp, HIGH);
  digitalWriteFast(SINJ, HIGH);
  digitalWriteFast(SHCH, HIGH);
  digitalWriteFast(SECY, HIGH);

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

  // Start thread
  threads.addThread(ctrlEthernetThread, 1);
  threads.addThread(heartBeatThread, 1);

  // Analog setup
  analogReadResolution(12); // set bits of resolution
}


void loop() {
  // operationMode selection
  static uint32_t previousSetting = 255;
  if (previousSetting != operationMode) {
    switch (operationMode) {
      case 1: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle1, 1000);
        }
        break;
      case 2: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle2, 1000);
        }
        break;
      case 3: {
          // The interval is specified in microseconds,
          // which may be an integer or floating point number,
          // for more highly precise timing.
          simulatedTiming.begin(simulatedCycle3, 1000);
        }
        break;
      default: {
          digitalWriteFast(TEN, LOW); // inhibit external timings

          simulatedTiming.end();
          operationMode = 0;
        }
        break;
    }
    Serial.print("Operation mode: ");
    Serial.println(operationMode);
    previousSetting = operationMode;
  }

  // read incoming timing on the interrupts
  noInterrupts();
  traceTime[0] = startOfcycle;
  traceTime[1] = calStart;
  traceTime[2] = calStop;
  traceTime[3] = harmonicChange;
  traceTime[4] = endOfCycle;
  interrupts();


  // Check buttons
  if (pushbutton1.update()) {
    if (pushbutton1.fallingEdge()) {
      operationMode++;
    }
  }
  if (pushbutton2.update()) {
    if (pushbutton2.fallingEdge()) {
      operationMode--;
    }
  }

  // Handle webServer
  handleWebServer();
}
