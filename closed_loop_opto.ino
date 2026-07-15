// ===================== Configuration =====================
const uint8_t NUM_CHANNELS = 3;

const uint8_t LICK_PINS[NUM_CHANNELS]  = {A5, A4, A3};    // lick TTL inputs, active HIGH
const uint8_t LED_PINS[NUM_CHANNELS]   = {5, 6, A2};  // LED drive outputs, active HIGH
const uint8_t DEBUG_PINS[NUM_CHANNELS] = {2, 3, 4}; // per-channel debug LEDs, active HIGH, mirror LED_PINS

const uint8_t STATUS_LED_PIN = 13; // onboard LED, on whenever any channel LED is on

const float PULSE_FREQ_HZ          = 10.0; // opto pulse train frequency (Hz)
const unsigned long PULSE_WIDTH_MS = 10;   // opto pulse ON time per cycle (ms)
const unsigned long EXTEND_MS      = 500;  // each lick extends the stim train by this much (ms)

const unsigned long PERIOD_MS = (unsigned long)(1000.0 / PULSE_FREQ_HZ);

// ===================== Per-channel state =====================
bool lastLickState[NUM_CHANNELS];
unsigned long trainEndTime[NUM_CHANNELS];
bool lastLedOn[NUM_CHANNELS];
bool lastAnyLedOn;

void setup() {
  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    pinMode(LICK_PINS[i], INPUT);
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    pinMode(DEBUG_PINS[i], OUTPUT);
    digitalWrite(DEBUG_PINS[i], LOW);
    lastLickState[i] = digitalRead(LICK_PINS[i]);
    trainEndTime[i] = 0;
  }
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(STATUS_LED_PIN, LOW);
}

void loop() {
  unsigned long now = millis();
  bool anyLedOn = false;

  for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
    bool lickState = digitalRead(LICK_PINS[i]);

    // rising edge = lick event; each lick resets the countdown, so licking at
    // >= 1000/EXTEND_MS Hz keeps the train running continuously
    if (lickState && !lastLickState[i]) {
      trainEndTime[i] = now + EXTEND_MS;
    }
    lastLickState[i] = lickState;

    bool trainActive = (long)(trainEndTime[i] - now) > 0;
    bool ledOn = false;

    if (trainActive) {
      unsigned long phase = now % PERIOD_MS;
      ledOn = phase < PULSE_WIDTH_MS;
    }

    if (ledOn != lastLedOn[i]) {
      digitalWrite(LED_PINS[i], ledOn);
      digitalWrite(DEBUG_PINS[i], ledOn);
      lastLedOn[i] = ledOn;
    }
    anyLedOn |= ledOn;
  }

  if (anyLedOn != lastAnyLedOn) {
    digitalWrite(STATUS_LED_PIN, anyLedOn);
    lastAnyLedOn = anyLedOn;
  }

  delayMicroseconds(50);
}
