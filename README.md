ir-switcher
===========

Send/receive command for switch on/off anything with IR-radiation.

Microcontroller: PIC 12f629 4Mhz, 
Languauge: C/Asm
Debugging: 1-line uart, small display based on PT6961 

Format of IR Command: TDB

experimental - some  experements to test 12f629 (include receiver based on asm)
ir-receiver.X - receiver receive and parse is signal. Show actual value of the signal on uart or on the test dispaly.
ir-transmitter.X - transmitter 
