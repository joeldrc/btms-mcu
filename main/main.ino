/**
 ******************************************************************************
  @file    BTMS mcu
  @author  Joel Daricou  <joel.daricou@cern.ch>
  @brief   web server ctrl I/O
 ******************************************************************************
  Board:
    - Developped on teensy 4.1
    - CPU speed: 600MHz
    - Optimize: Faster
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
#include <SPI.h>
#include <fnet.h>
#include "src\NativeEthernet\NativeEthernet.h"


/*** Global variables **/
uint8_t boardSN = 0;
uint8_t mac[] = { 0xDA, 0xAD, 0xBE, 0xEF, 0xFE, 0xE0 };

// 0: read only
// 1: simulate SCY and ECY
// 2: simulate SCY, INJ and ECY
// 3: simulate SCY, CALSTATR, CALSTOP, INJ and ECY
// 4: manual mode
int32_t operationMode = 0;
int32_t previousOperationMode = -1;

// enable type of simulation
bool continuousRunning = false;

volatile bool calStartSimulation = false;
volatile bool calStopSimulation = false;
volatile bool injSimulation = false;
volatile bool hchSimulation = false;

// timer variable for counting cycles
volatile uint8_t cnt_cycle = 0;

// digital values for the web page
volatile bool det10Mhz = 0;
volatile bool lock = 0;

// analog values for the web page
volatile float cpuTemp = 0;
volatile float v1 = 0;
volatile float v2 = 0;
volatile float v3 = 0;
volatile float v4 = 0;

// plot for web page
const uint32_t samplesNumber = 240; // 1200 milliseconds / 5 = 240 samples
const uint8_t numTraces = 6;
const char traceName[numTraces][10] = {{"SCY"}, {"CALSTRT"}, {"CALSTOP"}, {"INJ"}, {"HCH"}, {"ECY"}};

uint32_t traceTime[numTraces] = {0};
bool plot[numTraces][samplesNumber] = {0};

// Timer variables
const uint32_t pulseTime = 1;                 // time in uS

// Timing for each cycle
uint32_t scyTime = SCY_T;            // time in uS
uint32_t calstartTime = CALSTART_T;  // time in uS
uint32_t calstopTime = CALSTOP_T;    // time in uS
uint32_t injTime = INJ_T;            // time in uS
uint32_t hchTime = HCH_T;            // time in uS
uint32_t ecyTime = ECY_T;            // time in uS
uint32_t psTimeCycle = PSCYCLE_T;    // time in uS

// interrupt variables
volatile uint32_t startOfcycle = 0;
volatile uint32_t calStart = 0;
volatile uint32_t calStop = 0;
volatile uint32_t harmonicChange = 0;
volatile uint32_t endOfCycle = 0;


// button object
Bounce pushbutton1 = Bounce(SW6, 10);  // 10 ms debounce
Bounce pushbutton2 = Bounce(SW5, 10);  // 10 ms debounce


// timing object
elapsedMicros timing;

// timer object
IntervalTimer simulatedTiming;


// interrupt functions
FASTRUN void interrupt_ISCY() {
  timing = 1;
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


// mcu temperature
extern float tempmonGetTemp(void);


uint8_t readSettingSwitch(uint8_t pinA, uint8_t pinB) {
  // Read the current four-bit
  uint8_t b = 0;
  // The switch are active LOW
  if (digitalReadFast(pinA) == 0) b |= 1;
  if (digitalReadFast(pinB) == 0) b |= 2;
  //if (digitalReadFast(pinC) == 0) b |= 4;
  //if (digitalReadFast(pinD) == 0) b |= 8;
  return (b);
}


void ctrlEthernetThread() {
  while (1) {
    int linkStatusLed = ctrlConnection();
    switch (linkStatusLed) {
      case 1: {
          // connected (led green)
          digitalWriteFast(StsLedGr, LOW);
          digitalWriteFast(StsLedOr, HIGH);
        }
        break;
      case -1: {
          // unknown (led orange blinking)
          digitalWriteFast(StsLedGr, HIGH);
          digitalWriteFast(StsLedOr, LOW);
        }
        break;
      default: {
          // not connected (led off)
          digitalWriteFast(StsLedGr, HIGH);
          digitalWriteFast(StsLedOr, HIGH);
        }
        break;
    }
    ethernetConfig_thread();
    threads.delay(2000);
    threads.yield();
  }
}


void heartBeatThread() {
  while (1) {
    // Led shift
    static uint8_t ledCnt = 0;
    if (operationMode == 0) {
      switch (ledCnt) {
        case 0: {
            digitalWriteFast(StsLed1, HIGH);
            digitalWriteFast(StsLed2, LOW);
            digitalWriteFast(StsLed3, LOW);
            digitalWriteFast(StsLed4, LOW);
            ledCnt++;
          }
          break;
        case 1: {
            digitalWriteFast(StsLed1, HIGH);
            digitalWriteFast(StsLed2, HIGH);
            digitalWriteFast(StsLed3, LOW);
            digitalWriteFast(StsLed4, LOW);
            ledCnt++;
          }
          break;
        case 2: {
            digitalWriteFast(StsLed1, HIGH);
            digitalWriteFast(StsLed2, HIGH);
            digitalWriteFast(StsLed3, HIGH);
            digitalWriteFast(StsLed4, LOW);
            ledCnt++;
          }
          break;
        case 3: {
            digitalWriteFast(StsLed1, HIGH);
            digitalWriteFast(StsLed2, HIGH);
            digitalWriteFast(StsLed3, HIGH);
            digitalWriteFast(StsLed4, HIGH);
            ledCnt++;
          }
          break;
        default: {
            digitalWriteFast(StsLed1, LOW);
            digitalWriteFast(StsLed2, LOW);
            digitalWriteFast(StsLed3, LOW);
            digitalWriteFast(StsLed4, LOW);
            ledCnt = 0;
          }
          break;
      }
    }

    // Read digital inputs
    det10Mhz = digitalReadFast(_10MHzDet);
    lock = digitalReadFast(Lock);

    // Read analog values
    v1 = analogRead(ADC10);
    v2 = analogRead(ADC11);
    v3 = analogRead(ADC12);
    v4 = analogRead(ADC13);

    v1 = map(v1, 0, 4095, -3.15, 51.85);
    v2 = map(v2, 0, 4095, -21.6, 0);
    v3 = map(v3, 0, 4095, 0, 18.3);
    v4 = map(v4, 0, 4095, 0, 8.3);

    // read CPU temperature
    cpuTemp = tempmonGetTemp();

    threads.delay(240);
    threads.yield();
  }
}


void selectOperationMode() {
  // operationMode selection
  if (previousOperationMode != operationMode) {
    switch (operationMode) {
      case 1: {
          digitalWriteFast(TEN, LOW); // enable internal timing
          noInterrupts();
          cnt_cycle = 0;

          calStartSimulation = false;
          calStopSimulation = false;
          injSimulation = false;
          hchSimulation = false;
          interrupts();
          simulatedCycle();

          digitalWriteFast(StsLed5, LOW);
          digitalWriteFast(StsLed6, LOW);
          digitalWriteFast(StsLed7, LOW);
          digitalWriteFast(StsLed8, HIGH);
        }
        break;
      case 2: {
          digitalWriteFast(TEN, LOW); // enable internal timing
          noInterrupts();
          cnt_cycle = 0;

          calStartSimulation = false;
          calStopSimulation = false;
          injSimulation = true;
          hchSimulation = false;
          interrupts();
          simulatedCycle();

          digitalWriteFast(StsLed5, LOW);
          digitalWriteFast(StsLed6, LOW);
          digitalWriteFast(StsLed7, HIGH);
          digitalWriteFast(StsLed8, HIGH);
        }
        break;
      case 3: {
          digitalWriteFast(TEN, LOW); // enable internal timing
          noInterrupts();
          cnt_cycle = 0;

          calStartSimulation = true;
          calStopSimulation = true;
          injSimulation = true;
          hchSimulation = false;
          interrupts();
          simulatedCycle();

          digitalWriteFast(StsLed5, LOW);
          digitalWriteFast(StsLed6, HIGH);
          digitalWriteFast(StsLed7, HIGH);
          digitalWriteFast(StsLed8, HIGH);
        }
        break;
      case 4: {
          digitalWriteFast(TEN, LOW);   // disable external timings
          simulatedTiming.end();

          noInterrupts();
          calStartSimulation = false;
          calStopSimulation = false;
          injSimulation = false;
          hchSimulation = false;
          interrupts();

          digitalWriteFast(StsLed1, LOW);
          digitalWriteFast(StsLed2, LOW);
          digitalWriteFast(StsLed3, LOW);
          digitalWriteFast(StsLed4, LOW);

          digitalWriteFast(StsLed5, HIGH);
          digitalWriteFast(StsLed6, HIGH);
          digitalWriteFast(StsLed7, HIGH);
          digitalWriteFast(StsLed8, HIGH);
        }
        break;
      default: {
          digitalWriteFast(TEN, HIGH);  // enable external timings
          simulatedTiming.end();

          digitalWriteFast(StsLed5, LOW);
          digitalWriteFast(StsLed6, LOW);
          digitalWriteFast(StsLed7, LOW);
          digitalWriteFast(StsLed8, LOW);
          operationMode = 0;
        }
        break;
    }
    //Serial.print("Operation mode: ");
    //Serial.println(operationMode);
    previousOperationMode = operationMode;
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

  // active low signals
  digitalWriteFast(TEN, HIGH);

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

  // Read hardware switch
  Serial.print("Setting switch 1,2: ");
  Serial.print(readSettingSwitch(SW4, SW3));
  Serial.println();

  Serial.print("Setting switch 3,4: ");
  Serial.print(readSettingSwitch(SW2, SW1));
  Serial.println();

  // Set board serial number
  boardSN = readSettingSwitch(SW4, SW3);

  // Set mac address
  mac[5] += boardSN;

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

  // Reset CALSTOP (to fix an hardware problem)
  digitalWriteFast(SCalStp, LOW);
  delayMicroseconds(pulseTime);
  digitalWriteFast(SCalStp, HIGH);
}


void loop() {
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

  selectOperationMode();

  // Handle webServer
  handleWebServer();
}
