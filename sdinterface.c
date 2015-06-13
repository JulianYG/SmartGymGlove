/*
 * sdinterface.c
 *
 *  Created on: Jun 4, 2013
 *      Author: Gaurav
 */

#include "msp430.h"
#include "sdinterface.h"
#include "integer.h"
#include "i2c.h"

unsigned int RCA;
unsigned int cardtype;
unsigned long int sectorsize;

void wait(unsigned int cycles)
{
	unsigned int i;
	for(i=0;i<cycles;i++)
		asm("nop");
}
void deselect(){
	//deselect card

	unsigned int data = RCA;
	
	RCA = 128;
	send_CMD7();
	RCA = data;
	data = read16(0x30);
	data = data | 0x0001;
	write16(0x30,data);
}

void set_address(unsigned char address){
	unsigned int i;

	CLR_ADDR_CLK;
	CLR_ADDR_CLR;

	SET_ADDR_CLR;
	CLR_ADDR_CLR;

	for(i=0;i<7;i++){
		if((address&0x80)==0x80)
			SET_ADDR_DATA;
		else
			CLR_ADDR_DATA;

		SET_ADDR_CLK;
		CLR_ADDR_CLK;
		address <<= 1;
	}
}

unsigned int read16(unsigned char address)
{
	unsigned int data;
	//address<<=1;
	set_address(address);

	CLR_BE;
	CLR_CS;
	CLR_RE;
	data = P5IN + P6IN*256;
	SET_RE;
	SET_CS;
	SET_BE;

	return data;
}

void write16(unsigned char address, unsigned int data){

	P5DIR = 0xFF;
	P6DIR = 0xFF;

	P5OUT = data & 0x00FF;
	P6OUT = (data & 0xFF00)>>8;

	set_address(address);
	CLR_BE;
	CLR_CS;

	CLR_WE;
	CLR_WE;
	SET_WE;
	SET_WE;

	SET_CS;
	SET_BE;

	P6DIR = 0;
	P5DIR = 0;
	P5OUT = 0xFF;
	P6OUT = 0xFF;
}
void write16_DMABlock(unsigned char *data){

	unsigned int i;
	P5DIR = 0xFF;
	P6DIR = 0xFF;

	for(i=0;i<512;i+=2)
	{
		P5OUT = data[i];
		P6OUT = data[i+1];

		CLR_WE;

		SET_WE;
	}

	P6DIR = 0;
	P5DIR = 0;
	P5OUT = 0xFF;
	P6OUT = 0xFF;
}

void change_sdclk(unsigned char MULT,unsigned char DIV)
{
    write16(0x2C,0); // Disable SD Clock

      write16(0xFA,0x4020+MULT-1);//write16(0xFA,0x4021); //PLL Register: enable PLL (0x40) , Multiplier B = 1, Fout = (B+1)*Fin = 25 Mhz
      write16(0xF6,0x0000+DIV); //write16(0xF6,0x0001); //PLL Register: divider = 1, Clock = 12/1 = 25MHz

      write16(0x2C,1);
      while((read16(0x2C) & 0x0002)==0); //Wait while PLL is not stable

      write16(0x2C,7); // Enable SD Clock
}
int write_cmd(unsigned int arg0,unsigned int arg1,unsigned int transfer,unsigned int cmd)
{
	unsigned int data,timer;
	write16(0x08,arg0);
	write16(0x0A,arg1);
	write16(0x0C,transfer);
	write16(0x0E,cmd);
    data = read16(0x30);

    timer = 0;
	while((data & 0x0001)!= 0x0001)
	{
		data = read16(0x32);
		if(data & 0x0001)
			return 1;
		data = read16(0x30);

		timer++;
		if(timer>500)
			return 1;
	}

	data = data | 0x0001;
	write16(0x30,data);
	return 0;
}
void sleep(void){
	 write16(0xF8,1);
	 P5DIR = 0;
	 P6DIR = 0;
	 P4DIR = 0;
	 
	 P3DIR &= ~BIT2;
	 P3DIR &= ~BIT4;
	 P3DIR &= ~BIT5;
	 P3DIR &= ~BIT6;
	 P3DIR &= ~BIT7;

	 PJSEL &= ~BIT1; //Disable MCLK
	 PJDIR &= ~BIT1;

	P8DIR &= ~BIT3; // Turn off supply to SDIO, CPLD and image sensor
	P8OUT &= ~BIT3;
}

int init_card(unsigned int multi_factor){
	unsigned int data,timer,n;
	unsigned char csd[16];
	unsigned long int csz;

	PJSEL |= BIT1; // Enable MCLK line going to ImageSensor
    PJDIR |= BIT1;

	P3DIR = P3DIR | BIT2|BIT4|BIT5|BIT6|BIT7;
	P4DIR = BIT0| BIT1|BIT2|BIT3|BIT7;
	P5DIR = 0;
	P6DIR = 0;

	P5OUT = 0xFF; 	P6OUT = 0xFF; 	P5REN = 0xFF;	P6REN = 0xFF;
	P4REN = DREQ_PIN; P4OUT = P4OUT | DREQ_PIN;

	P3SEL = P3SEL & ~BIT2 & ~BIT4 & ~BIT5 & ~BIT6 & ~BIT7; 	P4SEL = 0; 	P5SEL = 0; 	P6SEL = 0;

    PJSEL |= BIT1; // Enable MCLK line going to ImageSensor
    PJDIR |= BIT1;

	P8DIR |= BIT3;
	P8SEL &= ~BIT3;

	P8OUT |= BIT3;

	PJSEL &= ~BIT2; 	PJDIR &= ~BIT2; 	PJOUT &= ~BIT2;

	P4DS = 0xFF;	P5DS = 0xFF;	P6DS = 0xFF;	P3DS = 0xFF;	P7DS = 0xFF;

	SET_BE; SET_WE; SET_RE; SET_CS; CLR_A8;

	SET_RESET;
    wait(5000);
	CLR_RESET;
    wait(200);

    write16(0x50,0x0001);

    change_sdclk(1,40); // Fout = 12*1/40 = 300 Khz

    write16(0x2C,1);

    timer = 0;
    data = read16(0x2C);
	while((data & 0x02)==0)
	{
		  write16(0x2C,1);
	    data = read16(0x2C);
	    timer++;
	    if(timer==TIMERTHRESH)
	    	return 1;
	}
    // Check internal clock = stable

    write16(0x34,0x01FF); //interrupt status enable register
    write16(0x36,0x03FF); //interrupt status enable register
    write16(0x38,0x01FF); //interrupt signal enable register
    write16(0x3A,0x03FF); //interrupt signal enable register

    write16(0xF4,0x09FF);
	write16(0xF8,2);
   // Enable SD Card Clock

	write16(0x2C,5);

	write16(0x28,0x0F00); // 1 Bit data bus, 3.3V signaling

	write_cmd(0,0,0,0);

       if(send_CMD8()==1)
    		   return 1;
      RCA = 0;

       if(send_CMD55()==1)
    	   return 1;

       if(send_ACMD41()==1) return 1;
       data = read16(0x12);
       timer = 0;
       // Until card is not busy..
       while((data & 0x8000)!= 0x8000)
       {
    	   send_CMD55();
    	   send_ACMD41();
           data = read16(0x12);
           timer++;
   	       if(timer==TIMERTHRESH)
   	    		return 1;
       }

       send_CMD2();
       send_CMD3();
       RCA = read16(0x12);

       write16(0x2E,13); // 1 sec timout

		send_CMD9();
		 data = read16(0x32);
		cardtype = read16(0x1E);
		*(int *)&csd[0] = read16(0x10);
		*(int *)&csd[2] = read16(0x12);
		*(int *)&csd[4] = read16(0x14);
		*(int *)&csd[6] = read16(0x16);
		*(int *)&csd[8] = read16(0x18);
		*(int *)&csd[10] = read16(0x1A);
		*(int *)&csd[12] = read16(0x1C);
		*(int *)&csd[14] = read16(0x1E);

		if ((cardtype >> 6) == 1) {	/* SDv2? */
			csz = csd[14-9] + ((WORD)csd[14-8] << 8) + ((DWORD)(csd[14-7] & 63) << 16) + 1;
			sectorsize = csz << 10;
		} else {					/* SDv1 or MMCv3 */
			n = (csd[14-5] & 15) + ((csd[14-10] & 128) >> 7) + ((csd[14-9] & 3) << 1) + 2;
			csz = (csd[14-8] >> 6) + ((WORD)csd[14-7] << 2) + ((WORD)(csd[14-6] & 3) << 10) + 1;
			sectorsize = csz << (n - 9);
		}

       send_CMD7(); // selecting card for data transfer
       data = read16(0x32);

       send_CMD6();
       wait(100);

       write16(0x28,0x0F06); // 4 Bit data bus, 3.3V signaling
       change_sdclk(multi_factor,1); // Fout = 12*4/1 = 48 Khz

       send_CMD55();
       send_ACMD6();
    	send_CMD16();

    	return 0;
}

int write_multibuffer(unsigned int addr1,unsigned int addr2,unsigned int blksize,unsigned char *buffer){
	   unsigned int data,j,timer;

	   write16(0x04,0x0200); //Block size: 512 bytes

	   send_CMD55();
	  send_ACMD23(blksize);

	   write16(0x06,blksize); //Block size: 512 bytes

	   SET_RE;
	   SET_WE;

	   timer = 0;
	   while(send_CMD25(addr1,addr2)!=0) // Multiple data block write request
	   {
			timer++;
			if(timer>20)
				return 1;
	   }
	   //DMA data transfer

		 SET_A8;
		CLR_BE;
		CLR_CS;

	   for(j=0;j<blksize;j++)
	  	   {
		   	   while(DREQ!=DREQ_PIN){}

		   	   write16_DMABlock(&buffer[j*512]);//buffer[i]);
	  	   }

	   	CLR_A8;
		SET_BE;
		SET_CS;

	   data = read16(0x06);

	   while(data>0)
	   {
		   data = read16(0x06);
	   }

	   data = read16(0x2A);
	   data = data | 0x01;
	   write16(0x2A,data);

	   data = read16(0x30);
	   timer = 0;
	   while((data & 0x0002)!=0x0002)
	   {
		   data = read16(0x30);
           timer++;
   	       if(timer==TIMERTHRESH)
   	    		return 1;
	   }
	   data = read16(0x30);
		data = data | 0x002;
		write16(0x30,data);
		wait(1000);
	   send_CMD12();

	   data = read16(0x2E);
	   data = data | 0x0600;
	   write16(0x2E,data);
	   data = read16(0x2E);
	   timer = 0;
	   while((data & 0x0600)!=0)
	   {
		   data = read16(0x2E);
           timer++;
   	       if(timer==TIMERTHRESH)
   	    		return 1;
	   }
	  data = read16(0x2A);
	   data = data & (!0x01);
	   write16(0x2A,data);
	   return 0;
}

void write_buffer(unsigned int addr1,unsigned int addr2,unsigned int *buffer){
	   unsigned int data,i;

	   send_CMD55();
	   send_ACMD23(1);
       write16(0x04,0x0200); //Block size: 512 bytes
	   send_CMD24(addr1,addr2); // Data write request for 1 block

	   data = read16(0x24);
	   while((data & 0x0400)!=0x0400)
	   {
		   data = read16(0x24);
	   }

	   for(i=0;i<256;i+=2)
	   {
		   write16(0x20,i);//buffer[i]);
		   write16(0x22,i+1);//buffer[i+1]);
	   }

	   data = read16(0x30);
	   while((data & 0x0002)!=0x0002)
	   {
		   data = read16(0x32);
		   data = read16(0x24);
		   data = read16(0x30);
	   }
	   data = read16(0x30);
		data = data | 0x002;
		write16(0x30,data);
}

int read_buffer(unsigned int addr1,unsigned int addr2,unsigned char *buffer,unsigned short sofs,unsigned short count){
		unsigned int data,i,offset,timer;
		offset = 0;

		write16(0x04,0x0200); //Block size: 512 bytes
		timer = 0;
		while(send_CMD17(addr1,addr2)!=0)
		{
			timer++;
			if(timer>20)
				return 1;
		}
		SET_WE;
		SET_A8;

		while(DREQ!=DREQ_PIN);

		CLR_BE;
		CLR_CS;

		for(i=0;i<512;i++)
		{
			CLR_RE;
			if(i>=sofs && offset<count)
			{
				buffer[offset] = P5IN;
				offset ++;
			}
			i++;
			if(i>=sofs && offset<count)
			{
				buffer[offset] = P6IN;
				offset ++;
			}
			SET_RE;
		}

		SET_CS;
		SET_BE;
		CLR_A8;

		data = read16(0x30);
		timer = 0;
		while((data & 0x0002)!=0x0002)
		{
			data = read16(0x30);
	           timer++;
	   	       if(timer==TIMERTHRESH)
	   	    		return 1;
		}
		   data = read16(0x30);
			data = data | 0x002;
			write16(0x30,data);

			return 0;
}

int read_multibuffer(unsigned int addr1,unsigned int addr2,unsigned char *buffer,unsigned char count){
		unsigned int data,i,cnt,j,timer;

		cnt = 0;
		write16(0x04,0x0200); //Block size: 512 bytes
		write16(0x06,count); //Block size: 512 bytes

		timer = 0;
		while((send_CMD18(addr1,addr2))!=0)
		{
			timer++;
			if(timer>20)
				return 1;
		}

		SET_WE;
		SET_A8;

		for(j=0;j<count;j++)
		{

			while(DREQ!=DREQ_PIN);
			
			CLR_BE;
			CLR_CS;

				for(i=0;i<512;i+=2)
				{
					CLR_RE;
					buffer[cnt++] = P5IN;
					buffer[cnt++] = P6IN;
					SET_RE;
				}

		}

		SET_CS;
		SET_BE;
		CLR_A8;
	   data = read16(0x2A);
	   data = data | 0x01;
	   write16(0x2A,data);

	   data = read16(0x30);
	   timer = 0;
	   while((data & 0x0002)!=0x0002)
	   {
		   data = read16(0x32);
		   data = read16(0x30);
		   timer++;
		   if(timer==TIMERTHRESH)
				return 1;
	   }
	   data = read16(0x32);
	   data = read16(0x30);
		data = data | 0x002;
		write16(0x30,data);

	   send_CMD12();

	   data = read16(0x2E);
	   data = data | 0x0600;
	   write16(0x2E,data);
	   data = read16(0x2E);
	   timer = 0;
	   while((data & 0x0600)!=0)
	   {
		   data = read16(0x2E);
		   timer++;
		   if(timer==TIMERTHRESH)
				return 1;
	   }
	   data = read16(0x2A);
	   data = data & (!0x01);
	   write16(0x2A,data);
	   return 0;
}

unsigned int write_multibuffer_camera(unsigned int addr1,unsigned int addr2){
	   unsigned int data,len,timer;

	   write16(0x04,0x0200); //Block size: 512 bytes

	   send_CMD55();
	   send_ACMD23(4096);

	   write16(0x06,4096); //Block size: 512 bytes
	   SET_RE;
	   SET_WE;

	   timer = 0;
	   while(send_CMD25(addr1,addr2)!=0) // Multiple data block write request
	   {
			timer++;
			if(timer>20)
				return 1;
	   }
	   //DMA data transfer

			SET_A8;
	  	  CLR_BE;
	  	   CLR_CS;

	  	 while(DREQ!=DREQ_PIN);

		   regwrite(3,1,1,2);
	  		data = regread(4,1,1);
	  		while(data!=7)  { data = regread(4,1,1); }

	  				while((P4IN & BIT6)==0) {}
	  				while((P4IN & BIT6)!=0) {}

		  	 SET_FRM_CLR;

	  	 	while(FRM_START != FRM_START_PIN);// != FRM_START_PIN);

	  	 	while(FRM_END != FRM_END_PIN);
	  	 	CLR_FRM_CLR;

	  	 	for(len=0;len<256;len++)
	  	 	{
	  			CLR_WE;
	  			SET_WE;
	  	 	}

	  	 	while(DREQ==DREQ_PIN)
	  	 	{
	  			CLR_WE;
	  			SET_WE;
	  	 	}

	  	 	CLR_A8;
			SET_CS;
			SET_BE;

	   data = read16(0x06);
	   len = 4096 - data;

	   data = read16(0x2A);
	   data = data | 0x01;
	   write16(0x2A,data);

	   data = read16(0x30);
	   while((data & 0x0002)!=0x0002){
		   data = read16(0x30);
	   }
	   data = read16(0x30);
		data = data | 0x006;
		write16(0x30,data);

		   data = read16(0x2A);
		   data = data & (~0x01);
		   write16(0x2A,data);

	   send_CMD12();

	   data = read16(0x2E);
	   data = data | 0x0600;
	   write16(0x2E,data);
	   data = read16(0x2E);
	   while((data & 0x0600)!=0)
	   {
		   data = read16(0x2E);
	   }
	   return len;
}
