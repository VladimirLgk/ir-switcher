/* 
 * File:   lcd.h
 * Author: Vladimir Legky
 *
 * Created on 07 January 2014, 23:13
 */

#ifndef LCD_H
#define	LCD_H

#define DIGIT0 0
#define DIGIT1 2
#define DIGIT2 4
#define DIGIT3 6
#define DIGIT4 8
#define DIGIT5 10
#define DIGIT6 12

unsigned char to7hex(unsigned char val);
void printLcdInt(unsigned int data);
void lcdWrite(unsigned char data);
void lcdPuts(unsigned char address, unsigned char * data, unsigned char size);
void lcdCmd(unsigned char cmd);
void lcdClear(void);
void lcdInit();

#endif	/* LCD_H */

