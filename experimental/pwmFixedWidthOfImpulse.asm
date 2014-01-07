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
	movlw	3
	movwf	freq
	movlw	24				;StartFreq
	call	Send4Periods
	movlw	30
	call	Delay			; call with N period
	goto	sender

;tryNexFreq
	movlw	24
	call	SendPeriods		;send period
	movlw	30
	call	Delay			; call with N period
;------------------------
	movlw	3
	call	SendPeriods		;send period
	movlw	1
	call	Delay			; call with N period
;-----------------------------
	incf	freq,1
	movlw	20
	xorwf	freq,0
	btfss	STATUS,Z
	; goto tryNexFreq
;------------------------
	movlw	20				;StartFreq
	movwf	freq
	movlw	10
	call	Delay			; call with N period
	goto	MainProgramm

;delay procedure
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

Send4Periods
	movwf	countOfsends
nextSend4
	bsf		GPIO,GP2	;Switch on Led
	movf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto	$-1
	nop
	nop
	bcf		GPIO,GP2	;Switch on Led
	movf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto	$-1
	nop
	nop
	bsf		GPIO,GP2	;Switch on Led
    movf	freq,0
	movwf	temp
	decfsz	temp,1
	 goto	$-1
    nop
	nop
	bcf		GPIO,GP2	;Switch on Led
	movlw	2
	movwf	temp
	decfsz	temp,1
	 goto	$-1
	nop
	nop
	decfsz	countOfsends,1
	 goto	nextSend4
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



