    list	 p=12f629
	#include p12f629.inc

;   Set configuration bits using definitions from the include file, p16f84.inc
	__config _INTRC_OSC_NOCLKOUT & _PWRTE_OFF & _WDT_OFF & _CP_OFF & _CPD_OFF & _MCLRE_OFF

;variables  definition
	cblock 0x20
		delayCounter
		hiTimerValue
	endc
;Program source
    org     0
	goto Start

	org		0x4
	btfss	PIR1, TMR1IF
		retfie
	bcf		PIR1, TMR1IF
	movlw 0x4
	xorwf	GPIO,1
	retfie

    org     0x50
Start
Configure
	;calibrate internal oscillator
	bsf		STATUS, RP0 ;Bank 1
	call	0x3FF		;Get the cal value
	movwf	OSCCAL		;Calibrate

    ;configure GP2 as led driver
	bcf     STATUS,RP0  ;Bank 0
	clrf    GPIO        ;Init GPIO
	movlw   0x07        ;Set GP<2:0> to
	movwf   CMCON       ;digital IO

	;clear Timer1 and setup
	clrf  TMR1H
	clrf TMR1L
	bcf	 PIR1, TMR1IF
	movlw b'00110100'	; internal clock, prescaller 8, timer off
	movwf T1CON
    
	bsf     STATUS,RP0  ;Bank
	movlw	0x3b		;Set only GP2 as output, all other is input.
    movwf	TRISIO
	;configure interrupt
	bsf PIE1,TMR1IE
	bsf INTCON,PEIE
	bsf INTCON,GIE

	bcf		STATUS,RP0  ;Bank 0
	;start timer
	bsf T1CON,TMR1ON

;loop for switch on, switch off loop for 4Mhz speed
MainProgramm
	nop
	goto MainProgramm

	org     0x3ff
	dw 0x3434
	end



