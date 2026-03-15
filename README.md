# LockInAmplifier
Or as a TLA, **LIA**.  
Similar to an FM (Frequency Modulated) Receiver or PM (Phase Modulated) Receiver.  

## Description General
An Arduino Uno Project.  
Synthesizes an I and Q oscillator (square waves) using the ATMege328 counter 1, the 16 bit counter.  
Modulated an LED with a with the I square wave.  
An A2D measures a signal.  
The resulting signal is multiplied by the I and Q to produce demodulated I and Q 
The I and Q are combined to produce a Magnitude and Angle 
The Magnitude is a measure of the input signal at the oscillator frequency.  

## Description And An Application, Optical Transmission
I have prototyped an optical transmission system.  
A RED LED is driven through a resistor off of the LED_BUILTIN pin, D13.  
A RED LED is reverse biased and current drives the base of a PNP Darlington pare and the collector current goes through a gain set resistor to GND.  
The two LEDs face each other where the light can be obstructed by a sample, like my finger.  
The Voltage at the Collector / Gain Set Resistor is the raw Signal sampled by A0.  
The UNO reports Magnitude out the serial port.  
The Arduino IDE Serial Plotter allows time series display of the resulting magnitude.  

First use is at 450Hz and half integer multiple of the 60Hz AC line in the USA.  
This frequency produces immunity to 60Hz electric fields and ambient light flicker.  

### Bread Board, POC, (Proof Of Concept)
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />](<img width="600" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />)   
Green wires are ground. Red are +5V and signals are yellow.
The transmitting, output LED is lower right. The receiver sensor LED is lower center somewhat obstructed by the yellow wire.

### Waveforms on POC
The Proof Of Concept measured with oscilloscope.  
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/a59b9095-55a2-4b8a-abb5-63332a55ae61" />  ](<img width="824" height="646" alt="image" src="https://github.com/user-attachments/assets/a59b9095-55a2-4b8a-abb5-63332a55ae61" />  )    
Channel 1 is drive to the output LED. 2V/Div.  
Channel 2 is the raw Vout at the Collector and Gain Set Resistor. 1V/Div.  
The received signal, is a triangle wave because the load resistor is 10K (later a 2K) and the capacitance of the Darlington transistor and stray capacitance of the solderless bread board and input to A0 form a low pass filter. This APE* circuit Darlington circuit was fast to fabricate from parts available on hand.


### Example Serial Plot
Arduino IDE Serial Plot output.  
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/78afe0ec-d967-40c2-8332-7ec796a1db1a" />  ](<img width="786" height="493" alt="image" src="https://github.com/user-attachments/assets/78afe0ec-d967-40c2-8332-7ec796a1db1a" />  )  

Image annotated with IrfanView


## Glossary, References

* *APE for Available Parts Engineering
* Circuit inspiration from: https://wiki.analog.com/university/courses/electronics/electronics-lab-led-sensor#:~:text=As%20more%20photons%20hit%20the,a%20photodiode%20light%20sensor%20%2F%20detector.
* [Irfanview](https://www.irfanview.com/)
