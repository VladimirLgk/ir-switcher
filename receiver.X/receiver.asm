    list	 p=12f629
	#include p12f629.inc

;   Set configuration bits using definitions from the include file, p16f84.inc
	__config _INTRC_OSC_NOCLKOUT & _PWRTE_OFF & _WDT_OFF & _CP_OFF & _CPD_OFF & _MCLRE_OFF

;variables  definition
	cblock 0x20
		blinkCount
		delayCounter
		temp
		Delay50MsCounter
		DelayOutCounter
		DelayInCounter
		command
		dataSize
		lcdData:4
		byteToSend
		bitCount
		digitAddress
		firstDigit
		secondDigit
		irData:20
		onCounterLow
		onCounterHi
		offCounterLow
		offCounterHi
	endc
;constants
STB equ	GP1
DAT equ GP0
CLK equ GP5
SERIAL equ GP2
IRPORT equ GP3

DIGIT0 equ lcdData
DIGIT1 equ lcdData+.2
DIGIT2 equ lcdData+.4
DIGIT3 equ lcdData+.6
DIGIT4 equ lcdData+.8
DIGIT5 equ lcdData+.10
DIGIT6 equ lcdData+.12
;===============================================================================
; Reset vector
;==============================================================================
    org     0
    goto    Start

;===============================================================================
;Programm vector
;===============================================================================
	org     0x50
;===============================================================================
;Return value of 7 segment char.
;===============================================================================
getSymCode:
	addwf	PCL,F
	retlw	b'00111111' ; 0
	retlw	b'00000110'	; 1
	retlw	b'01011011'	; 2
	retlw	b'01001111'	; 3
	retlw	b'01100110'	; 4
	retlw	b'01101101'	; 5
	retlw	b'01111101'	; 6
	retlw	b'00000111'	; 7
	retlw	b'01111111'	; 8
	retlw	b'01101111'	; 9
	retlw	b'01110111'	; a
	retlw	b'01111100'	; b
	retlw	b'00111001'	; c
	retlw	b'01011110'	; d
	retlw	b'01111001'	; e
	retlw	b'01110001'	; f

getAsciiCode:
	addwf	PCL,F
	retlw	0x30
	retlw	0x31
	retlw	0x32
	retlw	0x33
	retlw	0x34
	retlw	0x35
	retlw	0x36
	retlw	0x37
	retlw	0x38
	retlw	0x39
	retlw	'a'
	retlw	'b'
	retlw	'c'
	retlw	'd'
	retlw	'e'
	retlw	'f'

;===============================================================================
; convert hex to 7 segment symbol
; input arg: W - value
; Output args in: firstDigit, secondDigit
;===============================================================================
valueToHex
	movwf temp
	andlw 0xf
	call getSymCode
	movwf secondDigit
	swapf temp, 0
	andlw 0xf
	call getSymCode
	movwf firstDigit
	return

;===============================================================================
; convert hex to ascii symbol
; input arg: W - value
; Output args in: firstDigit, secondDigit
;===============================================================================
valueToAsciiHex
	movwf temp
	andlw 0xf
	call getAsciiCode
	movwf secondDigit
	swapf temp, 0
	andlw 0xf
	call getAsciiCode
	movwf firstDigit
	return

;==============================================
; Clear  display data
;==============================================
clearLcdData
	movlw lcdData
	movwf FSR
nextClear
	clrf INDF
	incf  FSR,1
	movf  FSR,0
	xorlw  lcdData+14
	btfss	STATUS,Z
	  goto  nextClear
	return
;===============================================================================
; Display initialize
;===============================================================================
InitDisplay
	movlw	.4
	call	Delay
	clrf	dataSize
	movlw	b'01000000'	;Command 2: write data, increment address
	call	sendDisplayCommand
	nop
	movlw	b'11000000'	;Command 3: set address
	call	sendDisplayCommand
	nop
	call	clearDisplay
	nop
	movlw	b'10001000'	;Command 4: switch on display
	call	sendDisplayCommand
	return

;===============================================================================
;Sends
;===============================================================================
clearDisplay
	movlw 14
	movwf dataSize
	movlw	b'11000000'	;Command 3: set address
	bcf	GPIO, STB
	call sendOneByte
sendData0
	clrw
	call sendOneByte
	decf dataSize,1
	btfss	STATUS,Z
		goto sendData0
	bsf	GPIO, STB
	return

;===============================================================================
; print One byte in lcd in begin of line.
; W contains byte to print
;===============================================================================
printHexOnLcd
	call valueToHex
	call clearLcdData
	movf	firstDigit, 0
	movwf	DIGIT5
	movf	secondDigit, 0
	movwf	DIGIT6
	movlw .14
	movwf dataSize
	movlw b'11000000'
	call sendDisplayCommand
	return

;===============================================================================
; send One byte to the display data in specified address
; digitAddress is address of data
; dataToSend, dataToSend+1  is word to send
;===============================================================================
sendDataToDisplay
	movf digitAddress,0
	iorlw	b'11000000'	;Command 3: set address
	bcf	GPIO, STB
	call sendOneByte
	movf lcdData, 0
	call sendOneByte
	incf digitAddress
	movf lcdData+1, 0
	call sendOneByte
	incf digitAddress
	bsf	GPIO, STB
	return

;===============================================================================
;send command and different data to the dispaly
;command is locaed in W-register
;data is located from beginData
;size of data contains in dataSize
;===============================================================================
sendDisplayCommand
	bcf	GPIO, STB
	call sendOneByte
	movlw lcdData
	movwf FSR
sendData
	clrw
	iorwf dataSize,0
	btfsc STATUS,Z
		goto endOfSend
	movf INDF,0
	call sendOneByte
	incf FSR
	decf dataSize,1
	goto sendData
endOfSend
	bsf	GPIO, STB
	return

;===============================================================================
;send one byte to the dispaly
;===============================================================================
sendOneByte
	movwf byteToSend
	movlw .8
	movwf bitCount
sendBit
	rrf	byteToSend,1
	btfsc STATUS,C
	 bsf GPIO,DAT
	bcf GPIO, CLK
	bsf	GPIO, CLK
	bcf	GPIO, DAT
	decfsz bitCount,1
     goto sendBit
	return
;===============================================================================
;send one byte as uart, register W contains data to send. ;
;Signal pass through UART line.
;speed: 9600. 1 start bit, 1 stop bit  
;===============================================================================
sendToUart
	movwf byteToSend
	movlw .8
	movwf bitCount
	bcf	STATUS, C	;start bit, alway zero
	call sendOneBit
sendUartBit
	rrf	byteToSend,1
	call sendOneBit	;data bit
	decfsz bitCount,1
     goto sendUartBit
	bsf	STATUS, C
	call sendOneBit	;stop bit, alway one
	return
;===============================================================================
;send one bit on CLK line with specified delay
;===============================================================================
sendOneBit
	movlw 0x21
	movwf temp
	btfsc STATUS,C
	 bsf GPIO, SERIAL
	btfss STATUS,C
	 bcf GPIO, SERIAL
	decfsz	temp,1
	 goto $-1
	return
;===============================================================================
;Read IR and blinking subroutine
;===============================================================================
ReadIrPort
	bsf		GPIO,GP4	;Switch on Led
	nop
	btfss	GPIO,IRPORT
		goto $-2
	movlw	.3
	bcf		GPIO,GP4	;Switch ooff Led
	movlw	.5
	call	Delay
	return

;==============================================
; Clear  IR data
;==============================================
clearIrData
	movlw irData
	movwf FSR
nextCleari
	clrf INDF
	incf  FSR,1
	movf  FSR,0
	xorlw  irData+20
	btfss	STATUS,Z
	  goto  nextCleari
	return


;===============================================================================
;Read IR and blinking subroutine
;===============================================================================
ReverseIrPort
	call	clearIrData
	bsf		GPIO,GP4	;Switch on Led
	movlw .10
	movwf dataSize
	movlw irData
	movwf FSR
	btfss	GPIO,IRPORT  ;wait for first Hi  signal
	  goto $-1
	bcf		GPIO,GP4	;Switch on Led
calcOn
	clrf onCounterLow
	clrf onCounterHi
	clrf offCounterLow
	clrf offCounterHi
	btfss	GPIO,IRPORT  ;check for following Low signal
	  goto calcOff
	incf blinkCount,1	; increment on time
	btfsc	STATUS,Z	; if overload  then  this is timeout
	  goto endTransmition
	goto calcOn
calcOff
	btfsc	GPIO,IRPORT  ;check for following Hi signal
	  goto endTransmition
	incf temp,1			; increment off time
	btfsc	STATUS,Z	; if overload  then  this is timeout
	  goto endTransmition
	goto	calcOff
endTransmition
	;store data
	movf  blinkCount,0
	movwf INDF
	incf FSR,1
	movf  temp,0
	movwf INDF
	incf FSR,1
	;check for stop parsing
	decf dataSize,1
	btfsc STATUS,Z
	 return
	movf temp,1
	btfsc STATUS, Z
	 return
	movf blinkCount,1
	btfsc STATUS, Z
	 return
	goto calcOn
	

;===============================================================================
;delay subroutine
;===============================================================================
Delay
	movwf Delay50MsCounter
	call Delay50Ms
	decfsz Delay50MsCounter,1
	 goto $-2
	return

Delay50Ms
	movlw 0x1b
	movwf DelayOutCounter
loop1
	movlw 0xe7
	movwf DelayInCounter
loop2
	nop
	nop
	nop
	nop
	nop
	decfsz DelayInCounter,1
	 goto loop2
	decfsz DelayOutCounter,1
	 goto loop1
	return

;===============================================================================
; Main programm. cycle, invoking  previous defined procedures
;===============================================================================
Start
Configure
	;calibrate internal oscillator
	bsf		STATUS, RP0 ;Bank 1
	call	0x3FF		;Get the cal value
	movwf	OSCCAL		;Calibrate
    ;configure GP2 as IO led driver
	bcf     STATUS,RP0  ;Bank 0
	clrf    GPIO        ;Init GPIO
	movlw   0x07        ;Set GP<2:0> to
	movwf   CMCON       ;digital IO
    bsf     STATUS,RP0  ;Bank 1
    movlw	b'11001000'		;Set only GP0,GP1,GP2,GP5,GP4 as output, all other is input.
    movwf	TRISIO
	bcf		STATUS,RP0  ;Bank 0
	bsf		GPIO,STB
	bcf		GPIO,DAT
	bsf		GPIO,CLK	
	bsf		GPIO,SERIAL	;init uart line
;Introduction with blinking of red and green led order by order
	;call	IntroBlinking
	;call	InitDisplay
	clrf	blinkCount
workingLoop
	movlw	'\r'
	call	sendToUart
	movlw	'\n'
	call	sendToUart
	call	ReverseIrPort
	movlw	'C'
	call	sendToUart
	movf	dataSize, 0
	sublw	.10
	movwf	dataSize
	call	printHexOnUart
	movlw	'\r'
	call	sendToUart
	movlw	'\n'
	call	sendToUart
	;-------------print data
	movlw irData
	movwf FSR
nextValue
	movlw	'H'
	call	sendToUart
	movf  INDF,0
	call	printHexOnUart
	incf  FSR,1
	movlw	' '
	call	sendToUart
	movlw	'L'
	call	sendToUart
	movf  INDF,0
	call	printHexOnUart
	incf  FSR,1
	movlw	'\r'
	call	sendToUart
	movlw	'\n'
	call	sendToUart
	decf	dataSize,1
	btfss	STATUS,Z
	  goto	nextValue

	movlw .40
	call  Delay
	goto	workingLoop

printHexOnUart
	call	valueToAsciiHex
	movf	firstDigit, 0
	call	sendToUart
	movf	secondDigit, 0
	call	sendToUart
	return
	;call	ReadIrPort
	;incf	blinkCount,1
	;movfw	blinkCount
	;call	printHexOnLcd

	;movfw	blinkCount
	;call	valueToAsciiHex
	;movf	firstDigit, 0
	;call	sendToUart
	;movf	secondDigit, 0
	;call	sendToUart
	;movlw	0x20
	;call	sendToUart

	org     0x3ff
	dw 0x3430
	end



