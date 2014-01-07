    list	 p=12f629
	#include p12f629.inc

;   Set configuration bits using definitions from the include file, p16f84.inc
	__config _INTRC_OSC_NOCLKOUT & _PWRTE_OFF & _WDT_OFF & _CP_OFF & _CPD_OFF & _MCLRE_OFF

;variables  definition
	cblock 0x20
		delayCounter
		temp
		countOfsends
		freq
	endc
;Program source
    org     0
    goto    Start
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

    bsf     STATUS,RP0  ;Bank 1
    movlw	0x3b		;Set only GP2 as output, all other is input.
    movwf	TRISIO

	bcf		STATUS,RP0  ;Bank 0

;loop for switch on, switch off loop for 4Mhz speed
MainProgramm
sender
	movlw	1
	movwf	freq
	movlw	5
	call	Delay			; call with N period
tryNexFreq
	movlw	1
	call	SendPeriods		;send period
	incf	freq,1
	;incf	freq,1
	;incf	freq,1
	movlw	10
	xorwf	freq,0
	btfss	STATUS,Z
		 goto tryNexFreq	
	goto	sender

;Send untuned periods
SendPeriods
	movwf	countOfsends
nextSend
	bsf		GPIO,GP2	;Switch on Led
	movf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto $-1
	bcf		GPIO,GP2	;Switch on Led
	movf	freq, 0
	movwf	temp
	decfsz	temp,1
	 goto $-1
	decfsz	countOfsends,1
	 goto	nextSend
	return

SendPeriodsFixWidth
	movwf	countOfsends
nextSendPeriod
	bsf		GPIO,GP2	;Switch on Led
	movf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto	$-1
	nop
	nop
	bcf		GPIO,GP2	;Switch on Led
	movlw	1
	subwf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto	$-1
	nop
	nop
	decfsz	countOfsends,1
	 goto	nextSendPeriod
	return

Delay
	movwf	delayCounter
delayLoop
	movf	freq,0
	movwf	temp
	addwf	temp,1
	nop
	decfsz	temp,1
	 goto $-1
	decfsz delayCounter,1
	 goto delayLoop
	return

	org     0x3ff
	dw 0x3434
	end



