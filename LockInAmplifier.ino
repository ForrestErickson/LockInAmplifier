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

// --- COMPILE-TIME CONFIGURATION ---
#define TARGET_FREQ 450.0       // Rejection against 60Hz mains
//#define TARGET_FREQ 62.0       // 62Hz will create a 2Hz beat against 60Hz mains
#define PRESCALER 64.0
#define SERIAL_DECIMATION 200  // High decimation for stability. Use with 450 Hz.
//#define SERIAL_DECIMATION 10  // for testing beat note at 62 hz.
#define TIMER_CLOCK (16000000.0 / PRESCALER)

// Calculate Timer Constants (Performed by your PC at compile-time)
const uint16_t timer_top = (uint16_t)(TIMER_CLOCK / (2.0 * TARGET_FREQ)) - 1;
const uint16_t phase_q   = timer_top / 2;

// --- Filter Settings ---
float alpha = 0.05;            
float dc_alpha = 0.01;         

// Filter variables (Spike-proof Floating Point)
float I_Filt = 0;
float Q_Filt = 0;
float DC_Level = 512.0; 

void setup() {
  pinMode(9, OUTPUT);           // I-Signal (Reference)
  pinMode(10, OUTPUT);          // Q-Signal
  pinMode(LED_BUILTIN, OUTPUT); // Status LED

  // --- TIMER 1: Mode 12 (ICR1 as TOP) ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  // WGM 13 & 12 = Mode 12 (CTC with ICR1 as TOP)
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  
  // CS11 & CS10 = Prescaler 64
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  
  ICR1  = timer_top;    // Set Frequency
  OCR1A = 0;            // I-Phase (Reference)
  OCR1B = phase_q;      // Q-Phase (90 degree shift)
  
  // Enable Hardware Toggle on Pins 9 & 10
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  Serial.begin(2000000);
  while (!Serial);
}

void loop() {
  static int decimationCounter = 0;
  static uint8_t lastPinState = 0;

  // Poll Port B (D8-D13) for reference clock edges
  uint8_t currentPinState = PINB;

  if ((currentPinState ^ lastPinState) & ((1 << PINB1) | (1 << PINB2))) {
    lastPinState = currentPinState;

    float i_p = (currentPinState & (1 << PINB1)) ? 1.0 : -1.0;
    float q_p = (currentPinState & (1 << PINB2)) ? 1.0 : -1.0;

    // Sync Built-in LED to I-phase
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
      
      // 4. Noise Gate (Lowered for beat-note visibility)
      if (Mag < 0.5) Mag = 0;

      // 5. Serial Plotter Output
      Serial.print("m:0,M:150,Mag:");
      Serial.println(Mag);
      
      decimationCounter = 0; 
    }
  }
}

