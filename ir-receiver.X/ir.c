#include "ir.h"
#include "system.h"

#define HiMarkError 1
#define LowMarkError 2
#define LowBitError 3
#define HiBitError 4
#define ByteTimeout 5
#define WrongHeader 6
#define WrongData 7

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
