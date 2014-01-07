IntroBlinking
	movlw	.3
	call	BlinkRed
	movlw	.3
	call	BlinkGreen
	movlw	.3
	call	BlinkPair
	return

BlinkRed
	movwf	blinkCount
br_cycle
	bsf		GPIO,GP5	;Switch on Led
	movlw	.3
	call	Delay
	bcf		GPIO,GP5	;Switch on Led
	movlw	.3
	call	Delay
	decfsz blinkCount,1
	 goto  br_cycle
	return

BlinkGreen
	movwf	blinkCount
bg_cycle
	bsf		GPIO,GP4	;Switch on Led
	movlw	.4
	call	Delay
	bcf		GPIO,GP4	;Switch on Led
	movlw	.4
	call	Delay
	decfsz blinkCount,1
	 goto  bg_cycle
	return

BlinkPair
	movwf	blinkCount
bp_cycle
	bsf		GPIO,GP4	;Switch on Led
	bsf		GPIO,GP5	;Switch on Led
	movlw	.4
	call	Delay
	bcf		GPIO,GP4	;Switch on Led
	bcf		GPIO,GP5	;Switch on Led
	movlw	.4
	call	Delay
	decfsz blinkCount,1
	 goto  bp_cycle
	return
	
	;===============================================================================
;send command and different data to the dispaly
;command is locaed in W-register
;data is located from beginData
;size of data contains in dataSize
;===============================================================================
sendDisplayCommand
	bcf	GPIO, STB
	nop
	call sendOneByte
sendData
	nop
	clrw
	iorwf dataSize,0
	btfsc STATUS,Z
		goto endOfSend
	movf beginData, 0
	movwf FSR
	movf INDF,0
	call sendOneByte
	incf FSR
	decf dataSize,1
	goto sendData
endOfSend
	bsf	GPIO, STB
	nop
	return

;===============================================================================
; pass through all simbols  by one element
;===============================================================================	
	
	clrf	digitAddress
	clrf	dataToSend
	clrf	dataToSend+1
WorkingCycle
	call	sendDataToDisplay
	call	ReadIrPort

	movf	dataToSend+1,0
	xorlw	0x3f
	btfsc	STATUS,Z
	  goto NextAddress

	movf	dataToSend,0
	xorlw	0xff
	btfsc	STATUS,Z
	 goto	incHiPart

	bcf	STATUS,C
	rlf		dataToSend,1
	bsf		dataToSend,0
	goto	WorkingCycle
	
incHiPart
	bcf	STATUS,C
	rlf		dataToSend+1,1
	bsf		dataToSend+1,0
	goto	WorkingCycle

NextAddress
	incf	digitAddress,1
	incf	digitAddress,1
	clrf	dataToSend
	clrf	dataToSend+1
	goto	WorkingCycle