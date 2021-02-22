

// Interrupt variables
volatile uint64_t StartOfcycle = 0;
volatile uint64_t CalStart = 0;
volatile uint64_t CalStop = 0;
volatile uint64_t Injection = 0;
volatile uint64_t HarmonicChange = 0;
volatile uint64_t EndOfCycle = 0;


void countCycle() {
  if (StartOfcycle > 0) {
    Serial.println(millis());
  }
}
