
#include "msp430.h"

#define SDA BIT1
#define SCLK BIT0

#define WAITUNIT 10
void wait(unsigned int cycles);
void sccb_init();
void start();
void stop();
void sendbyte(unsigned char byte);
unsigned char receivebyte();
void sccb_write(unsigned char chipaddr,unsigned char addr,unsigned int byte);
unsigned int sccb_read(unsigned char chipaddr,unsigned char addr);
void regwrite(unsigned char offset,unsigned char ID,unsigned char bytesize,unsigned int byte);
unsigned int regread(unsigned char offset,unsigned char ID,unsigned char bytesize);
void I2Cbyte_write(unsigned char chipaddr,unsigned char addr,unsigned char byte);
unsigned char I2Cbyte_read(unsigned char chipaddr,unsigned char addr);
unsigned char I2Cbyte_read(unsigned char chipaddr,unsigned char addr);
