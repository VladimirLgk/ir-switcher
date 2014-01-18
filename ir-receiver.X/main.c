#include "system.h"
#include <htc.h>	
#include <stdio.h>
#include "ir.h"
#include "lcd.h"
#include "uart.h"

__CONFIG(FOSC_INTRCIO & PWRTE_OFF & WDTE_OFF & CPD_OFF & CP_OFF & MCLRE_OFF);

//store calibrated value for osc as retlw instruction in given address
const unsigned char osccallibrate @ 0x3FF = 0x3450;

#define VALUES 20
unsigned char highLevel[VALUES];
unsigned char lowLevel[VALUES];

unsigned char highItems;
unsigned char lowItems;
unsigned char levelIndex;

unsigned char lastHiTime;
unsigned char lastLowTime;

void configure(void)
{
    //configure GPIO as Led driver, Lcd Driver,
    //Uart output driver, Ir Sensor
    GPIO = 0;
    CMCON = 0x07;
    TRISIO = 0b11001000;

    //setup initial value
    STB = 1;
    DAT = 0;
    CLK = 1;
    SERIAL = 1;
}

unsigned char toHex(unsigned char v)
{
    if(v >= 10)
       v += 'A'-'0'-10;
    v += '0';
    return v;
}

#define IR_ON 0
#define IR_OFF 1

void measureIrSignal(void )
{
    GLED = 1;
    while(readIrSensor() == IR_OFF){}
    GLED = 0;

    highItems = 0;
    lowItems = 0;
    levelIndex = 0;
    unsigned char countHi;
    unsigned char countLo;
    while(1)
    {
        countHi = 0;
        while(readIrSensor() == IR_ON)
            countHi++;

        countLo = 0;
        while(readIrSensor() == IR_OFF && countLo < 250 )
            countLo++;

        if(levelIndex < VALUES)
            highLevel[levelIndex] = countHi;
        lastHiTime = countHi;
        highItems++;

        if(levelIndex < VALUES)
            lowLevel[levelIndex] = countLo;
        lastLowTime = countLo;
        lowItems++;

        levelIndex++;
        if(lastLowTime >= 250)
            break;
    }

}

void puts__(const char * s);
void measureOutput();
void main(void)
{
    configure();
    unsigned char count = 0;
    GLED = 1;
    GLED = 0;
    GLED = 1;
    GLED = 0;
    for(count = 0;count < 3;count++)
    {
        GLED = 1;
        readIrSensor();
        GLED = 0;
    }

    for(count = 0;count < 3;count++)
    {
        GLED = 1;
        readIrSensor();
        GLED = 0;
        readIrSensor();
    }

    while(1)
    {
       puts__("start measuring\r\n");
       __delay_ms(500);
       measureIrSignal();
       measureOutput();
       __delay_ms(500);
    }
}

void puts__(const char * s)
{
    while(*s)
       putch(*s++);
}

void measureOutput()
{
      unsigned char count = 0;
       putch('A');
       putch(':');
       printUartInt(levelIndex);
       putch(',');
       putch('l');
       putch(':');
       printUartInt(lowItems);
       putch(',');
       putch('h');
       putch(':');
       printUartInt(highItems);
       putch('\r');
       putch('\n');
       for(count = 0;count < VALUES;count++)
       {
           putch('H');
           putch(':');
           printUartInt(highLevel[count]);
           putch(',');
           putch('L');
           putch(':');
           printUartInt(lowLevel[count]);
           putch('\r');
           putch('\n');
       }
       putch('N');
       putch(':');
       printUartInt(lastHiTime);
       putch(',');
       putch('F');
       putch(':');
       printUartInt(lastLowTime);
       putch('\r');
       putch('\n');

}