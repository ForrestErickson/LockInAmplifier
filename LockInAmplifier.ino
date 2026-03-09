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


// --- Settings ---
#define SERIAL_DECIMATION 25  

volatile int I_Demod = 0;     
volatile int Q_Demod = 0;     // Added Q-Demod
volatile bool newData = false;
volatile int sample = 0; 

void setup() {
  pinMode(9, OUTPUT);          // I-Signal (OC1A)
  pinMode(10, OUTPUT);         // Q-Signal (OC1B)
  pinMode(LED_BUILTIN, OUTPUT); 

  // --- Timer 1 Configuration (450 Hz) ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  ICR1 = 277;                            
  OCR1A = 0;                             
  OCR1B = 138;                           
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  // --- Interrupt Configuration ---
  PCICR  |= (1 << PCIE0);   
  // Mask for both Pin 9 (PCINT1) and Pin 10 (PCINT2)
  PCMSK0 = (1 << PCINT1) | (1 << PCINT2); 
  
  Serial.begin(2000000);
  while (!Serial);
}

ISR(PCINT0_vect) {
  uint8_t pinState = PINB; 
  sample = analogRead(A0); 

  // Synchronous Rectification for I (Pin 9)
  if (pinState & (1 << PINB1)) {
    PORTB |= (1 << PORTB5);    // LED Sync with I
    I_Demod = sample; 
  } else {
    PORTB &= ~(1 << PORTB5);
    I_Demod = -sample;
  }

  // Synchronous Rectification for Q (Pin 10)
  if (pinState & (1 << PINB2)) {
    Q_Demod = sample;
  } else {
    Q_Demod = -sample;
  }
  
  newData = true;
}

void loop() {
  static int decimationCounter = 0; 

  if (newData) {
    newData = false; 

    if (++decimationCounter >= SERIAL_DECIMATION) {
      // Local copies to maintain data integrity during print
      int plotI = I_Demod;
      int plotQ = Q_Demod;
      int plotSample = sample;

      // Plotter Output
      Serial.print("Min:-1023,Max:1023,Sample:");
      Serial.print(plotSample);     
      Serial.print(",I:");
      Serial.print(plotI);
      Serial.print(",Q:");
      Serial.println(plotQ);
      
      decimationCounter = 0; 
    }
  }
}
