/* 
 * File:   ir.h
 * Author: Vladimir Legky
 *
 * Created on 07 January 2014, 23:11
 */

#ifndef IR_H
#define	IR_H

#define IRPREFIX 0x82

extern volatile unsigned char error;
extern volatile unsigned char count;

unsigned char readIrSensor();
unsigned char readMark();
unsigned char readData(unsigned char * irdata, unsigned char size);
unsigned char getIrData(void);

#endif	/* IR_H */

