#include "system.h"
#include <htc.h>	
#include <stdio.h>
#include "ir.h"
#include "lcd.h"
#include "uart.h"

__CONFIG(FOSC_INTRCIO & PWRTE_OFF & WDTE_OFF & CPD_OFF & CP_OFF & MCLRE_OFF);

//store calibrated value for osc as retlw instruction in given address
const unsigned char osccallibrate @ 0x3FF = 0x3450;

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

#define LCD_DEBUG
//#define UART_DEBUG
void uartErrorMessage(unsigned char error);
void uartOkMessage(unsigned char * data);
void lcdErrorMessage(unsigned char * msg, unsigned char error);
void lcdOkMessage(unsigned char * msg, unsigned char * data);
void calibrate(void);

void main(void)
{
    configure();    
#ifdef LCD_DEBUG
    lcdInit();
    unsigned char lcdMessage[8];
    unsigned char data[2];
#endif
    calibrate();
    while(1)
    {
       __delay_ms(50);
#ifdef LCD_DEBUG
      lcdMessage[0] = 0b10000000;
      lcdPuts(DIGIT0 , lcdMessage, 1);
#endif
#ifdef UART_DEBUG
       putch('\r');
       putch('\n');
#endif
       unsigned char error = getIrData(data);
       if(error == 0)
       {
        #ifdef LCD_DEBUG
                lcdOkMessage(lcdMessage,data);
        #endif
        #ifdef UART_DEBUG
                uartOkMessage(data);
        #endif
                //TBD: perform operations
       }
       else
       {
        #ifdef LCD_DEBUG
                lcdErrorMessage(lcdMessage, error);
        #endif
        #ifdef UART_DEBUG
                uartErrorMessage(error);
        #endif
       }
    }
}

void calibrate(void)
{
    GLED = 1;
    GLED = 0;
    GLED = 1;
    readIrSensor();
    GLED = 0;
    count = 0;
    GLED = 1;
    while(!readIrSensor() && count < 1)
        count++;
    GLED = 0;
    count = 0;
    GLED = 1;
    while(!readIrSensor() && count < 2)
        count++;
    GLED = 0;
    count = 0;
    GLED = 1;
    while(!readIrSensor() && count < 3)
        count++;
    GLED = 0;
    count = 0;
    GLED = 1;
    while(!readIrSensor() && count < 4)
        count++;
    GLED = 0;
}

void lcdOkMessage(unsigned char * msg, unsigned char * data)
{
    msg[0] = 0;
    msg[1] = 0b01110011; //R
    msg[2] = 0b00011000; //R symbol
    msg[3] = to7hex((data[0] >> 4) & 0xf);
    msg[4] = to7hex(data[0] & 0xf);
    msg[5] = to7hex((data[1] >> 4) & 0xf);
    msg[6] = to7hex(data[1] & 0xf);
    lcdPuts(DIGIT0 , msg, 7);
}

void lcdErrorMessage(unsigned char * msg, unsigned char error)
{
    msg[0] = 0; //reset wait mark
    msg[1] = 0b01111001; //E
    msg[2] = 0b00000000; //E symbol
    msg[3] = to7hex((error >> 4) & 0xf);
    msg[4] = to7hex(error & 0xf);
    msg[5] = to7hex((count >> 4) & 0xf);
    msg[6] = to7hex(count & 0xf);
    lcdPuts(DIGIT0,msg, 7);
}

void uartOkMessage(unsigned char * data)
{
    putch('I');
    putch('R');
    putch(':');
    putch(toHex((data[0] >> 4) & 0xf));
    putch(toHex(data[0] & 0xf));
    putch(' ');
    putch(toHex((data[1] >> 4) & 0xf));
    putch(toHex(data[1] & 0xf));
}

void uartErrorMessage(unsigned char error)
{
    putch('E');
    putch(':');
    printUartInt(error);
    putch(' ');
    printUartInt(count);
}