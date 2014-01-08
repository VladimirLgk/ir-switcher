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
//#define IRDELAY_uS 45

  void custom_delay(unsigned int value)
  {
      while(value--)
      {
          _nop();
          _nop();
      }
  }

void startIrTransaction()
{
    IROUT = 1;
    custom_delay(delays[StartHi]);//220*IRDELAY_uS);
    IROUT = 0;
    custom_delay(delays[StartLow]);//110*IRDELAY_uS);
}

void sendData(unsigned char data)
{
    unsigned char mask = 0x1;
    while(mask)
    {
        IROUT = 1;
        custom_delay(delays[BitShort]);//12*IRDELAY_uS);
        IROUT = 0;
        if(data & mask)
            custom_delay(delays[BitLong]);//36*IRDELAY_uS);
        else
            custom_delay(delays[BitShort]);//12*IRDELAY_uS);
        mask = mask << 1;
    }
}

void finishIrTransaction()
{
    IROUT = 1;
    custom_delay(delays[BitShort]);//12*IRDELAY_uS);
    IROUT = 0;
    custom_delay(delays[FinishLow]);//250*IRDELAY_uS);
}

void sendIrData(unsigned char val)
{
    startIrTransaction();
    sendData(IRPREFIX);
    sendData((unsigned char)(~IRPREFIX));
    sendData(val);
    sendData((unsigned char)(~val));
    finishIrTransaction();
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

void fillDelays(unsigned int factor)
{
    delays[StartHi] = factor*20;
    delays[StartLow] = factor*10;
    delays[BitShort] = factor;
    delays[BitLong] =  factor*3;
    delays[FinishLow] = factor*20;
    printf("Delays: %d, %d, %d, %d, %d \r\n", delays[StartHi], delays[StartLow],
           delays[BitShort],delays[BitLong], delays[FinishLow] );
}

void main(void)
{
    configure();

    printf("Start ...\r\n");
    while(1)
    {
        unsigned int IRDELAY_uS = 27;
        while(IRDELAY_uS <= 28)
        {
            fillDelays(IRDELAY_uS);
            unsigned char counter = 3;
            while(counter--)
            {
                unsigned char value = 0xA8+counter;
                LED = 1;
                sendIrData(value);
                LED = 0;
                __delay_ms(500);
                printf("Ir value: %x, delay %d\r\n",value ,IRDELAY_uS);
            }
            IRDELAY_uS++;
        }
    }
}

