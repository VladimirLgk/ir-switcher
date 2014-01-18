/* 
 * File:   ir.h
 * Author: Vladimir Legky
 *
 * Created on 07 January 2014, 23:11
 */

#ifndef IR_H
#define	IR_H

#define IRPREFIX 0x82

#define HiPmbLength 210
#define LowPmbLength 100 
#define shortSignalLength 13
#define longSignalLength 40

unsigned char readIrSensor();
unsigned char readMark();
unsigned char readData(unsigned char * irdata, unsigned char size);
unsigned char getIrData(unsigned char *);
#endif	/* IR_H */

