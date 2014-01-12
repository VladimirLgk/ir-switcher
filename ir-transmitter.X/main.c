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
const unsigned char osccallibrate @ 0x3FF = 0x3434; //34

#define IRPREFIX 0x82

#define HiPmbLength 210
#define LowPmbLength 100
#define shortSignalLength 11
#define longSignalLength 36

#define DELAY_uS 35
#define CALLROUTINE_uS 10

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
    LED = 0;
    LED = 1;
     __delay_us(DELAY_uS);
    LED = 0;
    LED = 1;
    //us34Delay(0);
    //LED = 0;
    //LED = 1;
    us34Delay(1);
    LED = 0;
    LED = 1;
    us34Delay(2);
    LED = 0;
    LED = 1;
    us34Delay(3);
    LED = 0;
    LED = 1;
    us34Delay(4);
    LED = 0;
    LED = 1;
    us34Delay(5);
    LED = 0;
}

void main(void)
{
    configure();
    calibrate();
    unsigned char sendData = 0;
    unsigned char btn;
    unsigned char pressCount = 0;
    unsigned char prevBtn = 0;
    while(1)
    {
        prevBtn = btn;
        btn = readButtons();
        if(prevBtn == btn && btn != 0)
        {
            if( pressCount < 5 )
                pressCount++;
        }
        else
            pressCount = 0;

        if(btn == 1)
        {
            if(pressCount >= 5 )
                sendData++;
            else
                sendData = 0x1A;
        }
        else if(btn == 2)
        {
            if(pressCount >= 5 )
                sendData--;
            else
                sendData = 0x2B;
        }
        else if(btn == 3)
        {
            if(pressCount >= 5 )
                sendData = 0;
            else
                sendData = 0xff;
        }
        
        if(btn)
        {
            LED = 1;
            sendIrData(sendData);
            LED = 0;
            __delay_ms(80);
        }
    }
}

/*  sendData = 0;
       
            sendData = 0x56;*/

