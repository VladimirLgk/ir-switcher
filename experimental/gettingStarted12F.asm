    list	 p=12f629
	#include p12f629.inc

;   Set configuration bits using definitions from the include file, p16f84.inc
	__config _INTRC_OSC_NOCLKOUT & _PWRTE_OFF & _WDT_OFF & _CP_OFF & _CPD_OFF & _MCLRE_OFF

;variables  definition
startRam		EQU     0x20
LeastCounter    EQU     startRam+0
MostCounter     EQU     startRam+1

;Program source
    org     0
    goto    Start
    org     0x50

Start
	;calibrate internal oscillator
	bsf		STATUS, RP0 ;Bank 1
	call	0x3FF		;Get the cal value
	movwf	OSCCAL		;Calibrate

    ;configure GP2 as led driver
	bcf     STATUS,RP0  ;Bank 0
	clrf    GPIO        ;Init GPIO
	movlw   0x07        ;Set GP<2:0> to
	movwf   CMCON       ;digital IO

    bsf     STATUS,RP0  ;Bank 1
    movlw	0x3b		;Set only GP2 as output, all other is input.
    movwf	TRISIO

	bcf		STATUS,RP0  ;Bank 0

 ;loop for switch on, switch off loop for 4Mhz speed
SwithLoop
	bsf		GPIO,GP2	;Switch on Led
	call	Delay
	bcf		GPIO,GP2	;Switch off Led
	call	Delay
	goto	SwithLoop

;delay procedure
Delay
	movlw 0xff
	movwf LeastCounter
	call SubDelay
	decfsz LeastCounter,1
	 goto $-2
	return

SubDelay
	movlw 0xff
	movwf MostCounter
	nop
	decfsz MostCounter,1
	 goto $-2
	return

	org     0x3ff
	dw 0x3434
	end



