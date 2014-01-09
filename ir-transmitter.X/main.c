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

#define DELAY_uS 34
#define CALLROUTINE_uS 18

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

void main(void)
{
    configure();


    printf("Measure\r\n");
    LED = 1;
    LED = 0;
    LED = 1;
     __delay_us(DELAY_uS);
    LED = 0;
    LED = 1;
    us34Delay(1);
    LED = 0;
    LED = 1;
    us34Delay(1);
    LED = 0;
    LED = 1;
    us34Delay(1);
    LED = 0;
    printf("Start ...\r\n");
    while(1)
    {
        unsigned char counter = 32;
        while(counter--)
        {
            LED = 1;
            sendIrData(counter);
            LED = 0;
            __delay_ms(300);
            printf("Ir value: %x\r\n",counter);
        }
    }
}

