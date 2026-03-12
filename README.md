# LockInAmplifier
Or as a TLA, **LIA**.  
Similar to an FM (Frequency Modulated) Receiver or PM (Phase Modulated) Receiver.  

## Description General
An Arduino Uno Project.  
Synthisizers a I and Q oscillator (square waves) using the ATMege328 counter 1, the 16 bit counter.  
Modulated an LED with a with the I square wave.  
An A2D measures a signal.  
The resulting signal is multuplied by the I and Q to produce demodulated I and Q  
The I and Q are combined to produce a Magnatuce and Angle  
The Magnatude is a measure of the input signal at the oscillator frequency.  

## Description And An Application, Optical Transmission
I have prototyped an optical transmission system.  
A RED LED is driven through a resistor off of the LED_BUILTIN pin, D13.  
A RED LED is revirse biased and current drives the base of a PNP Darlington pare and the collector current goes through a gain set resistor to GND.  
The two LEDs face each other where the light can be obstruced by a sample, like my finger.  
The Voltage at the Collector / Gain Set Resitor is the raw Vsignal sampled by A0.  
The UNO reports Magnatude out the serial port.  
The Arduino IDE Serial Ploter allows time serise display of the resulting magnatude.  

First use is at 450Hz and half interger multipule of the 60Hz AC line in the USA.  
This frequency produces imunity to 60Hz electric fields and ambiant light flicker.  

### Bread Board, POC, (Proof Of Concept)
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />](<img width="600" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />)  
Green wires are ground. Red are +5V and signals are yellow.

### Waformes on POC
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />](<img width="600" alt="image" src="https://github.com/user-attachments/assets/5f6860a9-4cd5-49c2-90ec-61bf86803aa9" />)  
Channel 1 is drive to the output LED. 2V/Div.  
Channel 2 is the raw Vout at the Collector and Gain Set Resistor. 1V/Div.


### Example plot
[<img width="450" alt="image" src="https://github.com/user-attachments/assets/78afe0ec-d967-40c2-8332-7ec796a1db1a" />  ](<img width="786" height="493" alt="image" src="https://github.com/user-attachments/assets/78afe0ec-d967-40c2-8332-7ec796a1db1a" />  )  
Image annoted with IrfanView

