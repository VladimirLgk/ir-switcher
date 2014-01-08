#include "ir.h"
#include "system.h"

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7

//volatile unsigned char error;
//volatile unsigned char count;
//volatile unsigned char ir_mask;

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
/*
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
        while(readIrSensor() == 0 && count < 200)
            count++;

        //test is it zero
        if( (count >= 8 ) && ( count <= 14) )
        {
            value &= ~mask;
        }
        else if((count >= 24 ) && (count <= 44))  //test is it one
        {
            value |= mask;  
        }
        else if(count > 200)
        {
            if(mask == 0x80 && byte_count == 4 )
                break;
             return ByteTimeout;
        }
        else
            return HiBitError;

        if(mask == 0x80)
        {
            mask = 1;
            ir_mask = mask;
            byte_count++;
            *irdata = value;
            irdata++;
             value = 0;
            if(byte_count > size)
                break;
            continue;
        }

        mask = mask << 1;
        ir_mask = mask;
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
*/