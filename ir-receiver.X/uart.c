#include <htc.h>
#include <stdio.h>
#include "system.h"

#define UART_DELAY 96

void printUartInt(unsigned int data)
{
    unsigned int div = 1000;
    while(div)
    {
        unsigned char val = 0;
        if(data >= div)
            val = data/div;

        putch(val+0x30);
        data = data%div;
        div = div/10;
    }
}

void putch(unsigned char ch)
{
    unsigned char bit_count = 8;
    SERIAL = 0; //start bit, alway zero
    __delay_us(UART_DELAY);
    while(bit_count)
    {
        if((ch & 0x01))
           SERIAL = 1;
        else
           SERIAL = 0;
        ch = ch >> 1;
        bit_count--;
        __delay_us(UART_DELAY);
    }
    SERIAL = 1; //stop bit always one
    __delay_us(UART_DELAY);
}
