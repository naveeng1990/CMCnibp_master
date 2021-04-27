The NIBP main board is used to record data from 4 different sensors, such as
1. Cuff pressure
2. Intra-arterial pressure
3. 2 PPG signals

Hardware description
NIBP main board has
1. PIC microcontroller - start, stop, ramp up, transmits sensor signal to cmcdaq hardware
2. Two Pump(6V) and two valve to control cuff pressure
3. Self powered by Li--ION battery (3.6*2=7.4V) 
4. Battery charging circuit (Mobile phone charege micro-USB)
4. OLED display to show continuous cuff pressure
4. Start button(Green) to start the device and to do the ramp up operation
5. Stop button(rED) for emergency shutdown
6. CMCdaq software also controls the start and stop of data acquistion from sensors
