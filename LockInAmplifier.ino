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

/* 
 * Lock-In Amplifier - Interrupt Driven Version
 * Author: Forrest Lee Erickson (Modified for ISR Timing)
 */

#include <math.h>
#include <util/atomic.h> // For thread-safe reading of variables

// --- CONFIGURATION ---
#define TARGET_FREQ 450.0
#define PRESCALER 64.0
#define SERIAL_DECIMATION 200 
#define TIMER_CLOCK (16000000.0 / PRESCALER)

const uint16_t timer_top = (uint16_t)(TIMER_CLOCK / (2.0 * TARGET_FREQ)) - 1;
const uint16_t phase_q   = timer_top / 2;

// --- FILTER SETTINGS ---
float alpha = 0.05;            
float dc_alpha = 0.01;         

// Shared variables must be 'volatile' for ISRs
volatile float I_Filt = 0;
volatile float Q_Filt = 0;
volatile float DC_Level = 512.0; 
volatile bool newDataReady = false;

void setup() {
  pinMode(9, OUTPUT);           // I-Signal
  pinMode(10, OUTPUT);          // Q-Signal
  pinMode(LED_BUILTIN, OUTPUT);

  // Speed up ADC (Prescaler 16 instead of 128) for faster ISR execution
  ADCSRA = (ADCSRA & 0xf8) | 0x04; 

  cli(); // Disable interrupts during setup

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  // Mode 12 (CTC with ICR1 as TOP)
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler 64
  
  ICR1  = timer_top;    
  OCR1A = 0;            
  OCR1B = phase_q;      
  
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); // Toggle Pins 9 & 10

  // Enable Timer Compare Interrupts
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B); 

  sei(); // Enable interrupts

  Serial.begin(2000000);
}

// ISR for I-Phase (Pin 9 Toggle)
ISR(TIMER1_COMPA_vect) {
  processSample(PINB & (1 << PINB1), 1); // Reference I
}

// ISR for Q-Phase (Pin 10 Toggle)
ISR(TIMER1_COMPB_vect) {
  processSample(PINB & (1 << PINB2), 2); // Reference Q
}

// Helper function to handle math inside ISR
void processSample(bool pinHigh, int phaseType) {
  float ref = pinHigh ? 1.0 : -1.0;
  float s_raw = (float)analogRead(A0);
  
  // DC Rejection
  DC_Level += (s_raw - DC_Level) * dc_alpha;
  float s_ac = s_raw - DC_Level;

  // Demodulation
  if (phaseType == 1) {
    I_Filt += ((s_ac * ref) - I_Filt) * alpha;
    // Sync LED to I-phase
    if (pinHigh) PORTB |= (1 << PORTB5); else PORTB &= ~(1 << PORTB5);
  } else {
    Q_Filt += ((s_ac * ref) - Q_Filt) * alpha;
  }
}

void loop() {
  static int decimationCounter = 0;

  // Run decimation at a fixed rate
  if (++decimationCounter >= SERIAL_DECIMATION) {
    float currentI, currentQ;

    // Use ATOMIC_BLOCK to read volatile floats safely
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      currentI = I_Filt;
      currentQ = Q_Filt;
    }

    float Mag = sqrt(currentI * currentI + currentQ * currentQ);
    
    // Noise Gate
    if (Mag < 0.5) Mag = 0;

    Serial.print("m:0,M:150,Mag:");
    Serial.println(Mag);
    
    decimationCounter = 0; 
  }
  
  // Small delay to prevent loop from hogging CPU over the interrupts
  delay(1); 
}
