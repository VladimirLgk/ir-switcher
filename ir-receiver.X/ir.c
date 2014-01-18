#include "ir.h"
#include "system.h"

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7

unsigned char irdata[5];

//volatile unsigned char error;
volatile unsigned char count;
volatile unsigned char ir_mask;

#define IR_ON 0
#define IR_OFF 1

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
    do
    {
        GLED = 1;
        while(readIrSensor() == IR_OFF){}
        GLED = 0;

        //read preambula high
        count = 0;
        while(readIrSensor() == IR_ON)
            count++;
        if(count < (HiPmbLength-20) || count > (HiPmbLength+20))
            return HiMarkError;
         //read preambula low
        count = 0;
        while(readIrSensor() == IR_OFF)
            count++;

        if(count >= (longSignalLength+shortSignalLength-5) && count <= (longSignalLength+shortSignalLength+5) ) //repeat
            continue;
        else if(count >= (LowPmbLength-15) && count <= (LowPmbLength+15)) //data start
            break;
        else
            return LowMarkError;    //error
    }while(1);
    
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
        while(readIrSensor() == IR_ON)
            count++;
        if(count < (shortSignalLength-5) || count > (shortSignalLength+5))
            return HiBitError;

        count = 0;
        while(readIrSensor() == IR_OFF && count < (LowPmbLength+15))
            count++;
        if( (count >= (shortSignalLength-5) ) && ( count <= (shortSignalLength+5)))
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

unsigned char getIrData(unsigned char * outData)
{
    unsigned char error = readMark();
    if(! error)
    {
        error = readData(irdata, sizeof(irdata));
        if(! error)
        {
          if((irdata[0] == (unsigned char)~(irdata[1])))
          {
                if(irdata[2] == (unsigned char) ~(irdata[3]))
                {
                    outData[0] = irdata[0];
                    outData[1] = irdata[2];
                    return 0;
                }
                else
                   error = WrongData;
          }
          else
            error = WrongHeader;
        }
    }
    return error;
}
