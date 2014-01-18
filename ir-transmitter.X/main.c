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

#define IRPREFIX 0x17

#define HiPmbLength 145
#define LowPmbLength 73
#define shortSignalLength 9
#define longSignalLength 27

#define DELAY_uS 35

#define WAIT_38 12

void burst38khz(unsigned char value)
{
    while(value--)
    {
       _delay(WAIT_38);
       LED = 1;
       _delay(WAIT_38);
       LED = 0;
       _delay(WAIT_38);
       LED = 1;
       _delay(WAIT_38);
       LED = 0;
    }
}

void delay38khz(unsigned char value)
{
    while(value--)
    {
       _delay(WAIT_38);
       _nop();
       _delay(WAIT_38);
       _nop();
       _delay(WAIT_38);
       _nop();
       _delay(WAIT_38);
       _nop();
    }
}

void irPreamble()
{
    IROUT = 1;
    burst38khz(HiPmbLength);
    IROUT = 0;
    delay38khz(LowPmbLength);
}

void sendData(unsigned char data)
{
    unsigned char mask = 0x1;
    while(mask)
    {
        IROUT = 1;
        burst38khz(shortSignalLength);
        IROUT = 0;
        if(data & mask)
            delay38khz(longSignalLength);
        else
            delay38khz(shortSignalLength);
        mask = mask << 1;
    }
}

void irStopBit()
{
    IROUT = 1;
    burst38khz(shortSignalLength);
    IROUT = 0;
    delay38khz(LowPmbLength+50);
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

#define WAIT_38 12

void burst38khzCal(unsigned char value)
{
    while(value--)
    {
       _delay(WAIT_38);
       LED = 1;
       _delay(WAIT_38);
       LED = 0;
       _delay(WAIT_38);
       LED = 1;
       _delay(WAIT_38);
       LED = 0;
    }
}

void delay38khzCal(unsigned char value)
{
    while(value--)
    {
       _delay(WAIT_38);
       IROUT = 1;
       _delay(WAIT_38);
       IROUT = 0;
       _delay(WAIT_38);
       IROUT = 1;
       _delay(WAIT_38);
       IROUT = 0;
    }
}

void calibrate()
{
    LED = 1;
    LED = 0;
    LED = 1;
    LED = 0;
    LED = 1;
/*     __delay_us(DELAY_uS);
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
    //LED = 1;
    //us34Delay(4);
    //LED = 0;
    //LED = 1;
    //us34Delay(5);*/
    //LED = 0;
    _nop();
    _nop();
    _nop();
    burst38khzCal(3);
    burst38khzCal(3);
    _nop();
    _nop();
    _nop();
    burst38khzCal(6);
    _nop();
    _nop();
    LED = 1;
    delay38khzCal(4);
    LED = 0;
    LED = 1;
    delay38khzCal(3);
    LED = 0;
    LED = 1;
    delay38khzCal(2);
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

