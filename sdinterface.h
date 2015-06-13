/*
 * sdinterface.h
 *
 *  Created on: Jun 4, 2013
 *      Author: Gaurav
 */

#ifndef SDINTERFACE_H_
#define SDINTERFACE_H_

#define SET_BE P3OUT|=BIT2 //port3
#define CLR_BE P3OUT&=~BIT2

#define SET_WE P4OUT|=BIT7 //port4
#define CLR_WE P4OUT&=~BIT7

#define DREQ (P4IN & BIT5)
#define DREQ_PIN BIT5

#define FRM_START_PIN BIT6
#define FRM_START (P4IN & FRM_START_PIN)

#define FRM_END_PIN BIT4
#define FRM_END (P4IN & FRM_END_PIN)

#define SET_RE P3OUT|=BIT6 //port3
#define CLR_RE P3OUT&=~BIT6

#define SET_RESET P3OUT|=BIT5 //port3
#define CLR_RESET P3OUT&=~BIT5

#define SET_CS P3OUT|=BIT7 //port3
#define CLR_CS P3OUT&=~BIT7

#define SET_A8 P3OUT|=BIT4 //port3
#define CLR_A8 P3OUT&=~BIT4

#define SET_ADDR_CLK P4OUT|=BIT3 //port4
#define CLR_ADDR_CLK P4OUT&=~BIT3

#define SET_ADDR_CLR P4OUT|=BIT2 //port4
#define CLR_ADDR_CLR P4OUT&=~BIT2

#define SET_ADDR_DATA P4OUT|=BIT1 //port4
#define CLR_ADDR_DATA P4OUT&=~BIT1

#define SET_FRM_CLR P4OUT|=BIT0 //port4
#define CLR_FRM_CLR P4OUT&=~BIT0

#define TIMERTHRESH 10000

#define LED BIT6

extern unsigned int RCA;
extern unsigned int cardtype;
extern unsigned long int sectorsize;

#define send_CMD55() write_cmd(0,RCA,0,0x371A)
#define send_CMD8() write_cmd(0x01AA,0,0,0x081A)
#define send_ACMD41() write_cmd(0,0xD040,0,0x2902)
#define send_CMD2() write_cmd(0,0,0,0x0209)
#define send_CMD3() write_cmd(0,0,0,0x031A)
#define send_CMD7() write_cmd(0,RCA,0,0x071B)
#define send_CMD9() write_cmd(0,RCA,0,0x0909)

#define send_CMD24(addr1,addr2) write_cmd(addr1,addr2,0,0x183A) // Single block write command, Response type: R1
#define send_CMD25(addr1,addr2) write_cmd(addr1,addr2,0x0023,0x193A) // Multiple block write command, Response type: R1
#define send_CMD17(addr1,addr2) write_cmd(addr1,addr2,0x0011,0x113A) // Data read command, Response type: R1
#define send_CMD18(addr1,addr2) write_cmd(addr1,addr2,0x0033,0x123A) // Multiple block read command, Response type: R1; bug: doesn't work with command index enabled
#define send_CMD16() write_cmd(512,0,0,0x101A) // Set BlockLen:512
#define send_ACMD6() write_cmd(2,0,0,0x061A) // Set bus width to 4 bit, Response type R1
#define send_ACMD13() write_cmd(0,0,0,0x0D1A) // Set bus width to 4 bit, Response type R1
#define send_CMD6() write_cmd(0xFFF2,0x8000,0,0x061A)
#define send_ACMD23(numblock) write_cmd(numblock,0,0,0x171A) // pre erase blocks
#define send_CMD23(numblock) write_cmd(numblock,0,0,0x171A) // pre erase blocks
#define send_CMD12() write_cmd(0,0,0,0x0CDB) // Abort Command
#define send_CMD13() write_cmd(0,RCA,0,0x0D1A) // Send status command

void wait(unsigned int cycles);
void blinks();
void sleep();
void deselect();
void set_address(unsigned char address);
unsigned int read16(unsigned char address);
void write16(unsigned char address, unsigned int data);
void write16_DMABlock(unsigned char *data);
void change_sdclk(unsigned char MULT,unsigned char DIV);
int write_cmd(unsigned int arg0,unsigned int arg1,unsigned int transfer,unsigned int cmd);
int init_card(unsigned int multi_factor);
int write_multibuffer(unsigned int addr1,unsigned int addr2,unsigned int blksize,unsigned char *buffer);
unsigned int write_multibuffer_camera(unsigned int addr1,unsigned int addr2);
void write_buffer(unsigned int addr1,unsigned int addr2,unsigned int *buffer);
int read_buffer(unsigned int addr1,unsigned int addr2,unsigned char *buffer,unsigned short sofs,unsigned short count);
int read_multibuffer(unsigned int addr1,unsigned int addr2,unsigned char *buffer,unsigned char count);

#endif /* SDINTERFACE_H_ */
