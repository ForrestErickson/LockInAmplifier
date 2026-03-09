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

// We will use Pin Change Interrupts (PCINT) for Pins 9 and 10
// These share the same vector (PCINT0)

void setup() {
  // ... Keep all your Timer 1 setup code here ...

  // Setup Pin Change Interrupts on PB1 (D9) and PB2 (D10)
  PCICR  |= (1 << PCIE0);   // Enable PCINT0 group
  PCMSK0 |= (1 << PCINT1) | (1 << PCINT2); // Enable mask for D9 and D10
  
  Serial.begin(115200);
}

ISR(PCINT0_vect) {
  uint8_t pinState = PINB; // Read Port B state

  // Check Rising Edge on Pin 9 (I)
  if (pinState & (1 << PINB1)) { 
    captureI = analogRead(A0); 
    newI = true;
  }
  
  // Check Rising Edge on Pin 10 (Q)
  if (pinState & (1 << PINB2)) { 
    captureQ = analogRead(A0); 
    newQ = true;
  }
}

void loop() {
  if (newI && newQ) {
    // This is where you will implement Synchronous Demodulation
    // For now, we just print the raw I/Q samples
    Serial.print("I:"); Serial.print(captureI);
    Serial.print(" Q:"); Serial.println(captureQ);
    
    newI = false;
    newQ = false;
  }
}
