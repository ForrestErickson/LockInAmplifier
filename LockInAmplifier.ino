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


#define SERIAL_DECIMATION 100 
#define FILTER_SHIFT 6        

// Use 'long' for 32-bit precision to prevent overflow at 32,767
long I_Filtered = 0;
long Q_Filtered = 0;

void setup() {
  pinMode(9, OUTPUT);          
  pinMode(10, OUTPUT);         
  pinMode(LED_BUILTIN, OUTPUT); 

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  TCCR1B |= (1 << WGM13) | (1 << WGM12); 
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  ICR1 = 277;                            
  OCR1A = 0;                             
  OCR1B = 138;                           
  TCCR1A |= (1 << COM1A0) | (1 << COM1B0); 

  Serial.begin(2000000);
  while (!Serial);
}

void loop() {
  static int decimationCounter = 0;
  static uint8_t lastPinState = 0;

  uint8_t currentPinState = PINB;

  if ((currentPinState ^ lastPinState) & ((1 << PINB1) | (1 << PINB2))) {
    lastPinState = currentPinState;

    long i_phase = (currentPinState & (1 << PINB1)) ? 1L : -1L;
    long q_phase = (currentPinState & (1 << PINB2)) ? 1L : -1L;

    if (i_phase == 1L) PORTB |= (1 << PORTB5);
    else PORTB &= ~(1 << PORTB5);

    // Explicitly use long for the sample
    long s = (long)analogRead(A0);  //Dummy read for stabilization.
    s = (long)analogRead(A0); 
    
    // EMA Filter: (Sample * Phase * Scale) - current_average
    // Shifted by FILTER_SHIFT to update the moving average
    I_Filtered += ((s * i_phase * 256L) - I_Filtered) >> FILTER_SHIFT;
    Q_Filtered += ((s * q_phase * 256L) - Q_Filtered) >> FILTER_SHIFT;

    if (++decimationCounter >= SERIAL_DECIMATION) {
      // Print using fixed-length labels to save bytes
      // Serial.print("m:-1023,M:1023,I:");
      // Serial.print(I_Filtered / 256L);
      // Serial.print(",Q:");
      // Serial.println(Q_Filtered / 256L);
      
      Serial.println(I_Filtered);
      

      decimationCounter = 0; 
    }
  }
}
