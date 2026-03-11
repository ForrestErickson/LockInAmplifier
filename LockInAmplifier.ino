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

#include <math.h>

// --- Settings ---
#define SERIAL_DECIMATION 200  // Higher value for total system stability
float alpha = 0.05;            // Filter smoothness
float dc_alpha = 0.01;         // Ambient light tracking

// Filter variables as floats for "spike-proof" math
float I_Filt = 0;
float Q_Filt = 0;
float DC_Level = 512.0; 

void setup() {
  // Pin Setup
  pinMode(9, OUTPUT);           // I-Signal (Reference)
  pinMode(10, OUTPUT);          // Q-Signal
  pinMode(LED_BUILTIN, OUTPUT); // Status LED

  // --- TIMER 1: Mode 12 (ICR1 as TOP) - The 9-hour stable mode ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  // WGM 13 & 12 = Mode 12 (CTC with ICR1 as TOP)
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  
  // CS11 & CS10 = Prescaler 64
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  
  ICR1 = 277;    // Frequency: 450 Hz
  OCR1A = 0;     // Phase for Pin 9
  OCR1B = 138;   // Phase for Pin 10 (90 deg)
  
  // Enable Hardware Toggle on Pins 9 & 10
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  Serial.begin(2000000);
  while (!Serial);
}

void loop() {
  static int decimationCounter = 0;
  static uint8_t lastPinState = 0;

  // Poll hardware for reference clock edges
  uint8_t currentPinState = PINB;

  if ((currentPinState ^ lastPinState) & ((1 << PINB1) | (1 << PINB2))) {
    lastPinState = currentPinState;

    float i_p = (currentPinState & (1 << PINB1)) ? 1.0 : -1.0;
    float q_p = (currentPinState & (1 << PINB2)) ? 1.0 : -1.0;

    // Sync Built-in LED to Pin 9
    if (i_p > 0) PORTB |= (1 << PORTB5);
    else PORTB &= ~(1 << PORTB5);

    // 1. Capture and DC Rejection
    float s_raw = (float)analogRead(A0); 
    DC_Level += (s_raw - DC_Level) * dc_alpha;
    float s_ac = s_raw - DC_Level;

    // 2. Synchronous Demodulation
    I_Filt += ((s_ac * i_p) - I_Filt) * alpha;
    Q_Filt += ((s_ac * q_p) - Q_Filt) * alpha;

    if (++decimationCounter >= SERIAL_DECIMATION) {
      // 3. Magnitude Calculation
      float Mag = sqrt(I_Filt * I_Filt + Q_Filt * Q_Filt);
      
      // 4. Noise Gate: If signal is negligible, force to 0
      if (Mag < 1.5) Mag = 0;

      // 5. Serial Plotter Output (Minimal labels for stability)
      Serial.print("m:0,M:150,Mag:");
      Serial.println(Mag);
      
      decimationCounter = 0; 
    }
  }
}

