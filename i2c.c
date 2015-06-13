
#include "i2c.h"
#include "sdinterface.h"

void start(){
	//start of transmission

	P2OUT |= SDA;
	wait(WAITUNIT);
	P2OUT |= SCLK;
	wait(WAITUNIT);
	P2OUT &= ~SDA;
	wait(WAITUNIT);
	P2OUT &= ~SCLK;
	wait(WAITUNIT);
}
void stop(){
	//end of transmission

	P2OUT &= ~SDA;
	wait(WAITUNIT);
	P2OUT |= SCLK;
	wait(WAITUNIT);
	P2OUT |= SDA;
	wait(WAITUNIT);
	P2OUT &= ~SCLK;
	wait(WAITUNIT);
}
void sendbyte(unsigned char byte)
{
	unsigned int i;
	for(i=8;i>0;i--)
	{

		//ith bit
		if((byte>>(i-1))&0x01)
			P2OUT |= SDA;
		else
			P2OUT &= ~SDA;
		wait(WAITUNIT);

		P2OUT |= SCLK;
		wait(WAITUNIT);
		P2OUT &= ~SCLK;
		wait(WAITUNIT);
	}

	//Releasing the line for ACK bit

	P2DIR &= ~SDA;
	P2OUT |= SCLK;
	wait(WAITUNIT);

	//Checking ACK
	if((P2IN & SDA)!=0)
	{
		//No ACK received..
		_delay_cycles(WAITUNIT);
	//	while(1)
		{
		//	if((P2IN & SDA)==0)
			//	break;
		}
	}
	P2OUT &= ~SCLK;
	wait(WAITUNIT);
	P2DIR |= SDA;
}
void ACK()
{
	//Releasing the line for ACK bit
	P2OUT &= ~SDA;
	P2DIR |= SDA;
	wait(WAITUNIT);
	P2OUT |= SCLK;
	wait(WAITUNIT);
	P2OUT &= ~SCLK;
	wait(WAITUNIT);
}
void NACK(){
	//Releasing the line for ACK bit
	P2OUT |= SDA;
	P2DIR |= SDA;
	wait(WAITUNIT);
	P2OUT |= SCLK;
	wait(WAITUNIT);
	P2OUT &= ~SCLK;
	wait(WAITUNIT);
}
unsigned char receivebyte()
{
	unsigned int value,i;
	value = 0;
	//reading register value
	P2DIR &= ~SDA;
	for(i=8;i>0;i--){
		P2OUT |= SCLK;
		wait(WAITUNIT);

		value += (((P2IN & SDA)>>1)<<(i-1));

		P2OUT &= ~SCLK;
		wait(WAITUNIT);
	}
	return value;
}

void sccb_init(){
	P2SEL &= ~SDA;
	P2SEL &= ~SCLK;
	//set data direction = output;
	P2DIR = P2DIR | SDA | SCLK;
	P2DS = P2DS | SDA;
}

void sccb_write(unsigned char chipaddr,unsigned char addr,unsigned int byte){
	start();
	sendbyte(chipaddr); //Image sensor chip's address
	sendbyte(addr);
	sendbyte((byte&0xFF00)>>8);
	sendbyte(byte&0x00FF);
	stop();
}

unsigned int sccb_read(unsigned char chipaddr,unsigned char addr){
	unsigned int value;
	value = 0;
	start();
	sendbyte(chipaddr); //Image sensor chip's address
	sendbyte(addr);
	start();
	sendbyte(chipaddr+1); // Image sensor chip's address + Read bit
	value = (receivebyte()<<8) & 0xFF00;
	ACK();
	value = value + receivebyte();
	NACK();
	stop();
	return value;
}
void regwrite(unsigned char offset,unsigned char ID,unsigned char bytesize,unsigned int byte){
	unsigned int i;
	i = offset + ID*256 + (32768)*bytesize + 8192;
	sccb_write(0xBA,0xF0,1); //Selecting Page 1
	sccb_write(0xBA,0xC6,i); //Variable address
	sccb_write(0xBA,0xC8,byte); //Variable data
}
unsigned int regread(unsigned char offset,unsigned char ID,unsigned char bytesize){
	unsigned int i;
	i = offset + ID*256 + (32768)*bytesize + 8192;
	sccb_write(0xBA,0xF0,1); //Selecting Page 1
	sccb_write(0xBA,0xC6,i); //Variable address
	return(sccb_read(0xBA,0xC8));
}

void I2Cbyte_write(unsigned char chipaddr,unsigned char addr,unsigned char byte){
	start();
	sendbyte(chipaddr); //Accelerometer address 0x1D;
	sendbyte(addr);
	sendbyte(byte);
	stop();
}

unsigned char I2Cbyte_read(unsigned char chipaddr,unsigned char addr){
	unsigned char value;
	value = 0;
	start();
	sendbyte(chipaddr); //Battery monitoring chip's address 0x6C, touch controller address 0x36, accelerometer address 0x3A
	sendbyte(addr);
	start();
	sendbyte(chipaddr+1); //  chip's address + Read bit
	value = (receivebyte());
	ACK();
	stop();
	return value;
}
