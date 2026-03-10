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
#define FILTER_SHIFT 4        // Higher = smoother/slower. 4 is a good start.

volatile int I_Demod = 0;     
volatile int Q_Demod = 0;     
volatile bool newData = false;
volatile int rawSample = 0; 

// Filter variables (Longs to prevent overflow during intermediate math)
long I_Filtered = 0;
long Q_Filtered = 0;

void setup() {
  pinMode(9, OUTPUT);          
  pinMode(10, OUTPUT);         
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

  PCICR  |= (1 << PCIE0);   
  PCMSK0 = (1 << PCINT1) | (1 << PCINT2); 
  
  Serial.begin(2000000);
  while (!Serial);
}

ISR(PCINT0_vect) {
  uint8_t pinState = PINB; 
  int s = analogRead(A0); 
  rawSample = s;

  // Synchronous Rectification
  I_Demod = (pinState & (1 << PINB1)) ? s : -s;
  Q_Demod = (pinState & (1 << PINB2)) ? s : -s;
  
  // LED Sync with I
  if (pinState & (1 << PINB1)) PORTB |= (1 << PORTB5);
  else PORTB &= ~(1 << PORTB5);

  newData = true;
}

void loop() {
  static int decimationCounter = 0; 

  if (newData) {
    newData = false; 

    // --- Exponential Moving Average (EMA) Filter ---
    // We scale by 2^8 (256) internally to maintain precision without floats
    I_Filtered = I_Filtered + (((long)I_Demod * 256 - I_Filtered) >> FILTER_SHIFT);
    Q_Filtered = Q_Filtered + (((long)Q_Demod * 256 - Q_Filtered) >> FILTER_SHIFT);

    if (++decimationCounter >= SERIAL_DECIMATION) {
      // Scale back down for plotting
      int plotI = I_Filtered / 256;
      int plotQ = Q_Filtered / 256;

      Serial.print("Min:-1023,Max:1023,Raw:");
      Serial.print(rawSample);     
      Serial.print(",I_Filt:");
      Serial.print(plotI);
      Serial.print(",Q_Filt:");
      Serial.println(plotQ);
      
      decimationCounter = 0; 
    }
  }
}
