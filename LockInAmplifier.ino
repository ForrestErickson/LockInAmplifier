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
#define SERIAL_DECIMATION 100 // High decimation to give Serial hardware "breathing room"
#define FILTER_SHIFT 6        // Smooths out the 30Hz beat from 60Hz noise

long I_Filtered = 0;
long Q_Filtered = 0;

void setup() {
  pinMode(9, OUTPUT);          
  pinMode(10, OUTPUT);         
  pinMode(LED_BUILTIN, OUTPUT); 

  // --- Timer 1 (450 Hz Quadrature) ---
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  ICR1 = 277;                            
  OCR1A = 0;                             
  OCR1B = 138;                           
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  // NO INTERRUPTS - We will poll the hardware pins directly for max stability
  
  Serial.begin(2000000);
  while (!Serial);
}

void loop() {
  static int decimationCounter = 0;
  static uint8_t lastPinState = 0;

  // Poll the hardware pins (Port B) for any change
  uint8_t currentPinState = PINB;

  // If Pin 9 or Pin 10 has changed state...
  if ((currentPinState ^ lastPinState) & ( (1 << PINB1) | (1 << PINB2) )) {
    lastPinState = currentPinState;

    // Determine current phases
    int i_phase = (currentPinState & (1 << PINB1)) ? 1 : -1;
    int q_phase = (currentPinState & (1 << PINB2)) ? 1 : -1;

    // Sync LED to I-phase
    if (i_phase == 1) PORTB |= (1 << PORTB5);
    else PORTB &= ~(1 << PORTB5);

    // Sample and Rectify
    int s = analogRead(A0); 
    
    // EMA Filter with 8-bit internal precision
    I_Filtered += (((long)s * i_phase * 256) - I_Filtered) >> FILTER_SHIFT;
    Q_Filtered += (((long)s * q_phase * 256) - Q_Filtered) >> FILTER_SHIFT;

    // Decimated Output
    if (++decimationCounter >= SERIAL_DECIMATION) {
      Serial.print("Min:-1023,Max:1023,I_F:");
      Serial.print(I_Filtered / 256);
      Serial.print(",Q_F:");
      Serial.println(Q_Filtered / 256);
      
      decimationCounter = 0; 
    }
  }
}
