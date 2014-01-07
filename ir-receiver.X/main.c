#include <htc.h>	
#include <stdio.h>

#define _XTAL_FREQ 4000000

#define DAT GPIO0
#define STB GPIO1
#define CLK GPIO5
#define	LCD_STROBE()    ((CLK = 0),(CLK=1))

#define DIGIT0 0
#define DIGIT1 2
#define DIGIT2 4
#define DIGIT3 6
#define DIGIT4 8
#define DIGIT5 10
#define DIGIT6 12


#define SERIAL GPIO2
#define IRPORT GPIO3
#define GLED   GPIO4

__CONFIG(FOSC_INTRCIO & PWRTE_OFF & WDTE_OFF & CPD_OFF & CP_OFF & MCLRE_OFF);

//store calibrated value for osc as retlw instruction in given address
const unsigned char osccallibrate @ 0x3FF = 0x30;

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7

volatile unsigned char error;
volatile unsigned char count;

void lcd_write(unsigned char data)
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

void lcd_puts(unsigned char address, unsigned char * data, unsigned char size)
{
    STB = 0;
    lcd_write(0b11000000|address);
    if(data != NULL)
    {
        while(size--)
        {
            lcd_write(*data++);
            lcd_write(0);
        }
    }

    STB = 1;
}
void lcd_cmd(unsigned char cmd)
{
    STB = 0;
    lcd_write(cmd);
    STB = 1;
}

void lcd_clear(void)
{
    unsigned char count = 14;
    STB = 0;
    lcd_write(0b01000000);
    lcd_write(0b11000000);
    while(count)
    {
        lcd_write(0);
        count--;
    }
    STB = 1;
}

void lcd_init()
{
    __delay_ms(200);
    lcd_cmd(0b01000000); //Command 2: write data, increment address
    NOP();
    lcd_cmd(0b11000000); //Command 3: set address
    NOP();
    lcd_clear();
    lcd_cmd(0b10001000); //Command 4: switch on display
}

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

void putch(unsigned char ch)
{
    unsigned char bit_count = 8;
    SERIAL = 0; //start bit, alway zero
    __delay_us(98);
    while(bit_count)
    {
        if((ch & 0x01))
           SERIAL = 1;
        else
           SERIAL = 0;
        ch = ch >> 1;
        bit_count--;
        __delay_us(98);
    }
    SERIAL = 1; //stop bit always one
    __delay_us(98);
}

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

void printInt(unsigned int data)
{
    STB = 0;
    lcd_write(0b11000000 | DIGIT3 );

    unsigned int div = 1000;
    while(div)
    {
        unsigned char val = 0;
        if(data >= div)
            val = data/div;
	lcd_write(to7hex(val));
        data = data%div;
        div = div/10;
    }
    STB = 1;
}

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

unsigned char readIrSensor()
{
    unsigned char c=2;
    unsigned char port = IRPORT;
    do
    {
        //_nop();
        if( IRPORT != port)
            port = IRPORT;
        else
            c--;
    }
    while(c);
    return port;
}

unsigned char readMark()
{
    //wait till first changing on ir port
    GLED = 1;
    while(!readIrSensor()){}
    GLED = 0;

    //read first high mark
    count = 0;
    while(readIrSensor() == 1)
        count++;

    if(count < 180 || count > 225)
    {
        //ir_count = count;
        return HiMarkError;
    }

    count = 0;
    //read first low mark
    while(readIrSensor() == 0)
        count++;

    if(count < 90 || count > 115)
    {
        //ir_count = count;
        return LowMarkError;
    }
    return 0;
}

unsigned char readData(unsigned char * irdata, unsigned char size)
{
    unsigned char value = 0;
    unsigned char byte_count = 0;
    unsigned char mask = 1;
    value = 0;
    while(1)
    {
        count = 0;
        while(readIrSensor() == 1)
            count++;
        if(count < 8 || count > 14)
            return LowBitError;

        count = 0;
        while(readIrSensor() == 0)
            count++;

        //test is it zero
        if((count >=8 ) && ( count <=14) )
            value &= ~mask;
        else if(count > 33)  //test is it one
            value |= mask;
        else
            return HiBitError;

        if(count > 200 )
        {
            if(mask == 1)
            {
                byte_count--;
                break;
            }
            else
                return ByteTimeout;
        }

        if(mask == 0x80)
        {
            mask = 1;
            byte_count++;
            *irdata = value;
            irdata++;
             value = 0;
            if(byte_count > size)
                break;
            continue;
        }
       
        mask = mask << 1;
    }
    return 0;
}

unsigned char getIrData(void)
{
    unsigned char irdata[5];
    error = readMark();
    if(! error)
    {
        error = readData(irdata, sizeof(irdata));
        if(! error)
        {
          if((irdata[0] == (unsigned char)~(irdata[1])) &&
              (irdata[0] == 0x82))
          {
                if(irdata[2] == (unsigned char) ~(irdata[3]))
                {
                    return irdata[2];
                }
                else
                   error = WrongData;
          }
          else
            error = WrongHeader;
        }
    }
    return 0;
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
void main(void)
{
    configure();
    lcd_init();
    unsigned char lcd_data[8];
    while(1)
    {
       __delay_ms(500);
       error = 0;

#ifdef LCD_DEBUG
       lcd_data[0] = 0b10000000;
       lcd_puts(DIGIT0 , lcd_data, 1);
#endif
#ifdef UART_DEBUG
       putch('\r');
       putch('\n');
#endif
       unsigned char data = getIrData();
       if(error == 0)
       {
#ifdef LCD_DEBUG
            lcd_data[0] = 0;
            lcd_data[1] = 0b01110011; //R
            lcd_data[2] = 0b00011000; //R symbol
            lcd_data[3] = to7hex((data >> 4) & 0xf);
            lcd_data[4] = to7hex(data & 0xf);
            lcd_data[5] = 0;
            lcd_data[6] = 0;
            lcd_puts(DIGIT0 , lcd_data, 7);
#endif
#ifdef UART_DEBUG
           putch('I');
           putch('R');
           putch(':');
           putch(toHex((data >> 4) & 0xf));
           putch(toHex(data & 0xf));
#endif
       }
       else
       {
#ifdef LCD_DEBUG
            lcd_data[0] = 0; //reset wait mark
            lcd_data[1] = 0b01111001; //E
            lcd_data[2] = 0b00000000; //E symbol
            lcd_data[3] = to7hex((error >> 4) & 0xf);
            lcd_data[4] = to7hex(error & 0xf);
            lcd_data[5] = to7hex((count >> 4) & 0xf);
            lcd_data[6] = to7hex(count & 0xf);
            lcd_puts(DIGIT0,lcd_data, 7);
#endif
#ifdef UART_DEBUG
           putch('E');
           putch(':');
           printInt(error);
           putch(' ');
           printInt(count);
#endif

       }
    }
}

/*


     

       putch('\r');
       putch('\n');

       idx = 0;
       while(idx < byte_count)
       {
            putch('0');
            putch('b');
            unsigned char bitn = 0;
            while(bitn < 8)
            {
                if((irdata[idx] << bitn) & 0x80)
                     putch('1');
                else
                     putch('0');
                bitn++;
            }
            putch('\r');
            putch('\n');
            idx++;
       }*/