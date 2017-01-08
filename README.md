# Wemo-for-Alexa
This sketch is based on kakopappa Wemo switch and Shirriff's IR code library, to transmit commands for    
 1. Samsung TV    
 2. Marantz Amplifier    
 3. Miele VaccumCleaner
 4. IHC system
 5. Light Bulbs
More to come...
Some IR codes are RC5 and some are RAW.    
Wifi part (Witty) ESP8266 will receive commands from Alexa and parse them to the IR remote part.

The goal is to have a sketch which can send commands to different diveces such as TV, Radio, Amplifier, Light, Dimmers aso
from the the Witty ESP8266 with a IR diode attached (or a series).

Your are more than welcome to use the code, and change it to your needs. 
The project will be an ongoing process and ekstra devices will be added all the time. 
For now only switch on/off is working. Be aware of the amount of switches, it can only be 14 on one Witty.
