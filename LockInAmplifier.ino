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
 * Lock-In Amplifier - Interrupt Driven with LED PWM Indicators
 * Author: Forrest Lee Erickson (Modified for ISR Timing & PWM LEDs)
 */

#include <math.h>
#include <util/atomic.h> 

// --- CONFIGURATION ---
#define TARGET_FREQ 450.0
#define PRESCALER 64.0
//#define SERIAL_DECIMATION 200 
#define SERIAL_DECIMATION 25 
#define TIMER_CLOCK (16000000.0 / PRESCALER)

// PWM Pins for LEDs (Timer 0)
#define I_LED_PIN 5
#define Q_LED_PIN 6

const uint16_t timer_top = (uint16_t)(TIMER_CLOCK / (2.0 * TARGET_FREQ)) - 1;
const uint16_t phase_q   = timer_top / 2;

// --- FILTER SETTINGS ---
float alpha = 0.05;            
float dc_alpha = 0.01;         

volatile float I_Filt = 0;
volatile float Q_Filt = 0;
volatile float DC_Level = 512.0; 

void setup() {
  pinMode(9, OUTPUT);           // I-Reference Output
  pinMode(10, OUTPUT);          // Q-Reference Output
  pinMode(I_LED_PIN, OUTPUT);   // I-Component Magnitude LED
  pinMode(Q_LED_PIN, OUTPUT);   // Q-Component Magnitude LED
  pinMode(LED_BUILTIN, OUTPUT);

  // Speed up ADC for faster ISR execution
  ADCSRA = (ADCSRA & 0xf8) | 0x04; 

  cli(); 
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << WGM13) | (1 << WGM12); // Mode 12 (CTC)
  TCCR1B |= (1 << CS11) | (1 << CS10);   // Prescaler 64
  ICR1  = timer_top;    
  OCR1A = 0;            
  OCR1B = phase_q;      
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); // Toggle Pins 9 & 10
  TIMSK1 |= (1 << OCIE1A) | (1 << OCIE1B); // Enable Interrupts
  sei(); 

  Serial.begin(2000000);
}

ISR(TIMER1_COMPA_vect) {
  processSample(PINB & (1 << PINB1), 1); 
}

ISR(TIMER1_COMPB_vect) {
  processSample(PINB & (1 << PINB2), 2); 
}

// --- Update this function to include the safety check ---
void processSample(bool pinHigh, int phaseType) {
  float ref = pinHigh ? 1.0 : -1.0;
  float s_raw = (float)analogRead(A0);
  
  // Constrain s_raw to valid ADC range to prevent extreme math spikes
  s_raw = constrain(s_raw, 0, 1023);

  DC_Level += (s_raw - DC_Level) * dc_alpha;
  float s_ac = s_raw - DC_Level;

  if (phaseType == 1) {
    I_Filt += ((s_ac * ref) - I_Filt) * alpha;
    if (pinHigh) PORTB |= (1 << PORTB5); else PORTB &= ~(1 << PORTB5);
  } else {
    Q_Filt += ((s_ac * ref) - Q_Filt) * alpha;
  }

  // RECOVERY LOGIC: If a spike broke the math, reset filters to 0
  if (isnan(I_Filt)) I_Filt = 0;
  if (isnan(Q_Filt)) Q_Filt = 0;
  if (isnan(DC_Level)) DC_Level = 512.0;
}

void loop() {
  static int decimationCounter = 0;

  if (++decimationCounter >= SERIAL_DECIMATION) {
    float currentI, currentQ;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      currentI = I_Filt;
      currentQ = Q_Filt;
    }

    // 1. Calculate Magnitude for Serial Plotter
    float Mag = sqrt(currentI * currentI + currentQ * currentQ);
    if (Mag < 0.5) Mag = 0;

    // 2. Update PWM LEDs
    // Map absolute filter values to 0-255 PWM range.
    // Adjust '50.0' based on your expected signal amplitude.
    int i_brightness = constrain(abs(currentI) * 5.0, 0, 255);
    int q_brightness = constrain(abs(currentQ) * 5.0, 0, 255);
    
    analogWrite(I_LED_PIN, i_brightness);
    analogWrite(Q_LED_PIN, q_brightness);

    // 3. Serial Plotter Output
    Serial.print("m:0,M:150,Mag:");
    Serial.print(Mag);
    // Serial.print(",I_Val:");
    // Serial.print(abs(currentI));
    // Serial.print(",Q_Val:");
    // Serial.print(abs(currentQ));
    Serial.println();
    
    decimationCounter = 0; 
  }
  delay(1); 
}
