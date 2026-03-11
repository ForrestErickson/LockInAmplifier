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
//#define SERIAL_DECIMATION 100 
#define SERIAL_DECIMATION 10 
#define FILTER_SHIFT 6        
#define DC_TRACK_SHIFT 8     // Slow filter to track the 2.0V baseline

// Filter and Tracking variables (32-bit long to prevent overflow)
long I_Filt = 0;
long Q_Filt = 0;
long DC_Level = 512L * 256;  // Start at mid-scale (approx 2.5V)

void setup() {
  // Pin Setup
  pinMode(9, OUTPUT);          // I-Signal (OC1A) - To LED Driver
  pinMode(10, OUTPUT);         // Q-Signal (OC1B)
  pinMode(LED_BUILTIN, OUTPUT); // Status LED

  // --- Timer 1 Configuration (450 Hz Quadrature) ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  // Set WGM mode 12: CTC (Clear Timer on Compare Match)
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  
  // Set Prescaler to 64
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  
  ICR1 = 277;                            // Top for 449.6 Hz
  OCR1A = 0;                             // I-Phase Reference
  OCR1B = 138;                           // Q-Phase Reference (90 deg)
  
  // Enable Hardware Toggle on Pins 9 and 10
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  // Initialize Serial
  Serial.begin(2000000);
  while (!Serial);
  // Serial.println("Starting Stable Lock-In...");
}

void loop() {
  static int decimationCounter = 0;
  static uint8_t lastPinState = 0;

  // Poll Port B (D8-D13) for clock transitions
  uint8_t currentPinState = PINB;

  // Detect any edge on Pin 9 or Pin 10
  if ((currentPinState ^ lastPinState) & ((1 << PINB1) | (1 << PINB2))) {
    lastPinState = currentPinState;

    // Determine current phases
    long i_p = (currentPinState & (1 << PINB1)) ? 1L : -1L;
    long q_p = (currentPinState & (1 << PINB2)) ? 1L : -1L;

    // Sync Built-in LED to I-phase for visual verification
    if (i_p == 1L) PORTB |= (1 << PORTB5);
    else PORTB &= ~(1 << PORTB5);

    // Fast ADC Sample
    long s_raw = (long)analogRead(A0); 
    
    // 1. DC TRACKING: Update ambient baseline estimate
    DC_Level += ((s_raw * 256L) - DC_Level) >> DC_TRACK_SHIFT;
    
    // 2. DC REJECTION: Center the signal by subtracting the baseline
    long s_ac = s_raw - (DC_Level >> 8);

    // 3. DEMODULATION: Synchronous rectification of the centered signal
    I_Filt += ((s_ac * i_p * 256L) - I_Filt) >> FILTER_SHIFT;
    Q_Filt += ((s_ac * q_p * 256L) - Q_Filt) >> FILTER_SHIFT;

    // 4. PLOTTING: Decimate output to keep Serial buffer clear
    if (++decimationCounter >= SERIAL_DECIMATION) {
      
      // Calculate Vector Magnitude (Phase Independent Strength)
      float I_val = (float)(I_Filt >> 8);
      float Q_val = (float)(Q_Filt >> 8);
      float Magnitude = sqrt(I_val * I_val + Q_val * Q_val);

      // Serial Plotter output
      Serial.print("m:0,M:1000,Mag:");
      Serial.println((int)Magnitude);
      
      decimationCounter = 0; 
    }
  }
}
