/* 
 * File:   main.c
 * Author: Vladimir Legky
 *
 * Created on 08 January 2014, 01:20
 */
#include <htc.h>
#include <stdio.h>

#include "system.h"
#include "uart.h"

__CONFIG(FOSC_INTRCIO & PWRTE_OFF & WDTE_OFF & CPD_OFF & CP_OFF & MCLRE_OFF);

//store calibrated value for osc as retlw instruction in given address
const unsigned char osccallibrate @ 0x3FF = 0x34;
//volatile unsigned char IRDELAY_uS;
volatile unsigned int delays[5];

#define StartHi 0
#define StartLow 1
#define BitShort 2
#define BitLong 3
#define FinishLow 4

#define IRPREFIX 0x82

#define HiPmbLength 210
#define LowPmbLength 100
#define shortSignalLength 11
#define longSignalLength 36

#define DELAY_uS 43
#define CALLROUTINE_uS 14

void us34Delay(unsigned char value)
{
  __delay_us(DELAY_uS-CALLROUTINE_uS);
  value--;
  while(value--)
     __delay_us(DELAY_uS);
}

void irPreamble()
{
    IROUT = 1;
    us34Delay(HiPmbLength);
    IROUT = 0;
    us34Delay(LowPmbLength);
}

void sendData(unsigned char data)
{
    unsigned char mask = 0x1;
    while(mask)
    {
        IROUT = 1;
        us34Delay(shortSignalLength);
        IROUT = 0;
        if(data & mask)
            us34Delay(longSignalLength);
        else
            us34Delay(shortSignalLength);
        mask = mask << 1;
    }
}

void irStopBit()
{
    IROUT = 1;
    us34Delay(shortSignalLength);
    IROUT = 0;
    us34Delay(LowPmbLength+50);
}

void sendIrData(unsigned char val)
{
    irPreamble();
    sendData(IRPREFIX);
    sendData((unsigned char)(~IRPREFIX));
    sendData(val);
    sendData((unsigned char)(~val));
    irStopBit();
}

unsigned char readButtons(void)
{
    unsigned char c = 5;
    unsigned char prevButton = ((unsigned char) (BTNON) << 1) | (BTNOFF);
    unsigned char button = 0;
    do
    {
        __delay_ms(5);
        button = ((unsigned char) (BTNON) << 1) | (BTNOFF);
        if( prevButton != button)
            prevButton = button;
        else
            c--;
    }
    while(c);
    return button;
}

void configure(void)
{
    //configure GPIO as:
    //GPIO0 = out, uart line port, GPIO1 = input, N.C. ,
    //GPIO2 = out, IR transmitter, GPIO3 = input, Button On.
    //GPIO4 = input, Button Off, GPIO5 = out, View led Driver.
    GPIO = 0;
    CMCON = 0x07;
    TRISIO = 0b11011010;

    //setup initial value
    IROUT = 0;
    LED = 0;
    SERIAL = 1;
}

void calibrate()
{
    LED = 1;
    LED = 0;
    LED = 1;
     __delay_us(DELAY_uS);
    LED = 0;
    LED = 1;
    us34Delay(1);
    LED = 0;
    LED = 1;
    us34Delay(2);
    LED = 0;
    LED = 1;
    us34Delay(3);
    LED = 0;
}

void main(void)
{
    configure();
    printf("Measure\r\n");
    calibrate();
    printf("Start ...\r\n");
    unsigned char sendData;;
    unsigned char btn;
    while(1)
    {
        btn = readButtons();
        printf("Buttons: %d\r\n",btn );
        sendData = 0;
        if(btn == 1)
            sendData = 0x12;
        else if(btn == 2)
            sendData = 0x34;
        else if(btn == 3)
            sendData = 0x56;

        if(sendData != 0)
        {
            LED = 1;
            sendIrData(sendData);
            LED = 0;
            __delay_ms(300);
        }
    }
}

