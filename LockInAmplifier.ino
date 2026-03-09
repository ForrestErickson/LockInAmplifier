/* Lock In Amplifier.
Let us make a lock in amplifier where we use as the reference signal not sinusoids but square waves.
Similar to Sine and Cosine, we desire two Square Waves with 90 degree phase offset.
There are four phases, t=0, t=1, t=2 and t=3 or t0, t1, t2, t3.
We will call the In phase signal the square wave which transitions from -1 to 1 at t=0.
We will call the Quadrature phase signal the square wave which transitions from 1 to -1 at t=1.

Author: Forrest Lee Erickson
Date: 2260309

Asking an AI for design. 
Generate I and Q at 450 Hz.
Capture A2D on riseing edge of I and Q


*/


volatile int captureI = 0;
volatile int captureQ = 0;
volatile bool newI = false;
volatile bool newQ = false;

void setup() {
  pinMode(9, OUTPUT);          // I-Signal (OC1A)
  pinMode(10, OUTPUT);         // Q-Signal (OC1B)
  pinMode(LED_BUILTIN, OUTPUT); // Status LED

  // --- Timer 1 Configuration (450 Hz Quadrature) ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << WGM13) | (1 << WGM12); // CTC Mode
  TCCR1B |= (1 << CS11) | (1 << CS10);   // Prescaler 64
  ICR1 = 277;                            // Top for 450Hz
  OCR1A = 0;                             // I-Phase
  OCR1B = 138;                           // Q-Phase (90 deg)
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); // Hardware Toggle

  // --- Interrupt Configuration ---
  PCICR  |= (1 << PCIE0);   
  PCMSK0 |= (1 << PCINT1) | (1 << PCINT2); 
  
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Starting.");
}

ISR(PCINT0_vect) {
  uint8_t pinState = PINB; // Read Port B (Pins 8-13)

  // Sync LED_BUILTIN (Pin 13) with Pin 9 (I-Signal)
  if (pinState & (1 << PINB1)) {
    PORTB |= (1 << PORTB5);    // Set LED High
    captureI = analogRead(A0); // Sample I
    newI = true;
  } else {
    PORTB &= ~(1 << PORTB5);   // Set LED Low
  }
  
  // Check Rising Edge on Pin 10 (Q)
  if (pinState & (1 << PINB2)) { 
    captureQ = analogRead(A0); // Sample Q
    newQ = true;
  }
}

void loop() {
  if (newI && newQ) {
    // Restore the printout for the analog capture
    Serial.print("I:"); Serial.print(captureI);
    Serial.print(" Q:"); Serial.println(captureQ);

    // Reset flags for the next cycle
    newI = false;
    newQ = false;
  }
}
