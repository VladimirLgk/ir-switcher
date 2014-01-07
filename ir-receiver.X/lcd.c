//#include 
#include "lcd.h"
#include "system.h"

#define	LCD_STROBE() ((CLK = 0),(CLK=1))

unsigned char to7hex(unsigned char val)
{
    #pragma switch space
    switch(val)
    {
        case 0:
            return 0b00111111;
        case 1:
            return 0b00000110;
        case 2:
            return 0b01011011;
        case 3:
            return 0b01001111;
        case 4:
            return 0b01100110;
        case 5:
            return 0b01101101;
        case 6:
            return 0b01111101;
        case 7:
            return 0b00000111;
        case 8:
            return 0b01111111;
        case 9:
            return 0b01101111;
        case 10:
            return 0b01110111;
        case 11:
            return 0b01111100;
        case 12:
            return 0b00111001;
        case 13:
            return 0b01011110;
        case 14:
            return 0b01111001;
        case 15:
            return 0b01110001;
    }
}

void printLcdInt(unsigned int data)
{
    STB = 0;
    lcdWrite(0b11000000 | DIGIT3 );

    unsigned int div = 1000;
    while(div)
    {
        unsigned char val = 0;
        if(data >= div)
            val = data/div;
	lcdWrite(to7hex(val));
        data = data%div;
        div = div/10;
    }
    STB = 1;
}

void lcdWrite(unsigned char data)
{
   unsigned char bitCount = 8;
   while(bitCount)
   {
       if(data & 0x1)
          DAT = 1;
       else
          DAT = 0;
       LCD_STROBE();
       DAT = 0;
       data = data >> 1;
       bitCount--;
   }
}

void lcdPuts(unsigned char address, unsigned char * data, unsigned char size)
{
    STB = 0;
    lcdWrite(0b11000000|address);
    if(data != 0)
    {
        while(size--)
        {
            lcdWrite(*data++);
            lcdWrite(0);
        }
    }

    STB = 1;
}

void lcdCmd(unsigned char cmd)
{
    STB = 0;
    lcdWrite(cmd);
    STB = 1;
}

void lcdClear(void)
{
    unsigned char count = 14;
    STB = 0;
    lcdWrite(0b01000000);
    lcdWrite(0b11000000);
    while(count)
    {
        lcdWrite(0);
        count--;
    }
    STB = 1;
}

void lcdInit()
{
    __delay_ms(200);
    lcdCmd(0b01000000); //Command 2: write data, increment address
    NOP();
    lcdCmd(0b11000000); //Command 3: set address
    NOP();
    lcdClear();
    lcdCmd(0b10001000); //Command 4: switch on display
}


