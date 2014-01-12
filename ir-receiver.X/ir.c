#include "ir.h"
#include "system.h"

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7

volatile unsigned char error;
volatile unsigned char count;
volatile unsigned char ir_mask;

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
    //read preambula high
    count = 0;
    while(readIrSensor() == 1)
        count++;
    if(count < (HiPmbLength-20) || count > (HiPmbLength+20))
        return HiMarkError;
     //read preambula low
    count = 0;
    while(readIrSensor() == 0)
        count++;
    if(count < (LowPmbLength-15) || count > (LowPmbLength+15))
        return LowMarkError;

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
        if(count < (shortSignalLength-3) || count > (shortSignalLength+3))
            return HiBitError;

        count = 0;
        while(readIrSensor() == 0 && count < (LowPmbLength+15))
            count++;
        if( (count >= (shortSignalLength-2) ) && ( count <= (shortSignalLength+2)))
            value &= ~mask; //test and set for zero bit
        else if((count >=  (longSignalLength-5) ) && (count <= (longSignalLength+5)))
            value |= mask;  //test and set for one bit
        else if(count >= (LowPmbLength+15))
            break;          //test for finish transaction
        else
            return LowBitError;

        if(mask == 0x80)
        {
            irdata[byte_count] = value;
            byte_count++;
            mask = 1;
            value = 0;
            if(byte_count >= size)
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
              (irdata[0] == IRPREFIX))
          {
                if(irdata[2] == (unsigned char) ~(irdata[3]))
                    return irdata[2];
                else
                   error = WrongData;
          }
          else
            error = WrongHeader;
        }
    }
    return 0;
}
