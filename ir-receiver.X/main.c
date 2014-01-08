#include <htc.h>	
#include <stdio.h>

#include "system.h"
#include "ir.h"
#include "lcd.h"
#include "uart.h"

__CONFIG(FOSC_INTRCIO & PWRTE_OFF & WDTE_OFF & CPD_OFF & CP_OFF & MCLRE_OFF);

//store calibrated value for osc as retlw instruction in given address
const unsigned char osccallibrate @ 0x3FF = 0x30;

#define VALUES 17
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

void measureIrSignal(void )
{
    GLED = 1;
    while(!readIrSensor()){}
    GLED = 0;

    highItems = 0;
    lowItems = 0;
    levelIndex = 0;
    unsigned char countHi;
    unsigned char countLo;
    while(1)
    {
        countHi = 0;
        while(readIrSensor() == 1)
            countHi++;
     
        countLo = 0;
        while(readIrSensor() == 0 && countLo < 250 )
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

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7


#define HiPmbLength 210
#define LowPmbLength 100
#define shortSignalLength 11
#define longSignalLength 36

unsigned char count;
unsigned char count_bit;
unsigned char mask;
unsigned char byte_count;

unsigned char countBitSignal(void )
{
    unsigned char value = 0;
    byte_count = 0;
    mask = 1;
    GLED = 1;
    while(!readIrSensor()){}
    GLED = 0;

    //read first high mark
    count = 0;
    while(readIrSensor() == 1)
        count++;
    if(count < (HiPmbLength-20) || count > (HiPmbLength+20))
        return HiMarkError;

    count = 0;
    while(readIrSensor() == 0)
        count++;
    if(count < (LowPmbLength-15) || count > (LowPmbLength+15))
        return LowMarkError;

    value = 0;
    count_bit = 0;
    while(1)
    {
        count = 0;
        while(readIrSensor() == 1)
            count++;
        if(count < (shortSignalLength-3) || count > (shortSignalLength+3))
            return HiBitError;

        count = 0;
        while(readIrSensor() == 0 && count < 250)
            count++;

        //test is it zero
        if( (count >= (shortSignalLength-2) ) && ( count <= (shortSignalLength+2)) )
        {
            value &= ~mask;
        }
        else if((count >=  (longSignalLength-5) ) && (count <= (longSignalLength+5)))
        {//test is it one
            value |= mask;
        }
        else if(count >= 250)
        {
            break;
        }
        else
        {
            return LowBitError;
        }
        count_bit++;
        if(mask == 0x80)
        {
            mask = 1;
            highLevel[byte_count] = value;
            byte_count++;
            value = 0;
            if(byte_count > VALUES)
                break;
            continue;
        }
        mask = mask << 1;
    }
    
    return 0;
}

unsigned char highItems;
unsigned char lowItems;
unsigned char levelIndex;

unsigned char lastHiTime;
unsigned char lastLowTime;

void measureOutput();
void countBitErrorOutput(unsigned char error_);
void countBitInfoOutput();

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
       putch('\r');
       putch('\n');
       __delay_ms(500);
       measureIrSignal();
       measureOutput();

       __delay_ms(500);
       unsigned char error_ = countBitSignal();
       if(error_ != 0)
       {
           countBitErrorOutput(error_);
       }
       else
       {
           countBitInfoOutput();
       }
    }
}

void puts__(const char * s)
{
    while(*s++)
       putch(*s);
}

void countBitErrorOutput(unsigned char error_)
{
    puts__("\r\nErr: ");
    printUartInt(error_);
    puts__(", Times: ");
    printUartInt(count);
    puts__(", Bits: ");
    printUartInt(count_bit);
    puts__(", Mask: ");
    printUartInt(mask);
    puts__(", Bytes: ");
    printUartInt(byte_count);
    puts__("\r\n");
}

void countBitInfoOutput()
{
    unsigned char c;
    puts__("\r\nBytes: ");
    printUartInt(byte_count);
    puts__(", Bites: ");
    printUartInt(count_bit);
    puts__(", Times: ");
    printUartInt(count);
    puts__(", Mask: ");
    printUartInt(mask);
    putch('\r');
    putch('\n');
    puts__("Data: ");
    for(c=0; c < byte_count; c++)
    {
        putch( toHex( (highLevel[c] >> 4) & 0x0f ));
        putch( toHex( highLevel[c]&0x0f));
        putch(' ');
    }
    puts__("\r\n");
}

void measureOutput()
{
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