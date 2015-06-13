/* Turbine Spiro setup */
#include "msp430.h"
#include <in430.h>
#include <stdio.h>
#include "comm.h"
#include "defs.h"
#include "i2c.h"
#include "ff.h"
#include "sdinterface.h"
#include "stdlib.h"
#include "string.h"

#define LIPOLY
#define TOUCHRESET BIT0
#define TOUCHCHANGE BIT0

unsigned char xdata,ydata,zdata,pdata;  // unsigned char xdata,ydata,zdata,pdata; // changed from char xdata;
unsigned char temp_x[500],temp_y[500],temp_z[500],temp_p[500];
unsigned char data_x[10],data_y[10],data_z[10],data_p[10];
char xfile[8],yfile[8],zfile[8],pfile[8];	// store the names of new created files; has to be 8, why? pass the pointer but...
unsigned int data_counter;
unsigned int temp_counter;

unsigned char Buff[2048]; //unsigned
unsigned char store;
unsigned char interrupt_flag;
unsigned int buffer_flag;

PARTITION VolToPart[1];
FATFS Fatfs;			/* File system object */
FRESULT rc;
FIL Fil,Fil1;			/* File object */
DIR dir;

void die(FRESULT rc) {if(rc) return;}

void lowbatt(void){

    while(1){

    	P7OUT &= ~BIT6;
    	_delay_cycles(300000);
    	P7OUT |= BIT6;
    	_delay_cycles(300000);}
}

void turbine_setupTA3(void){
	//first stop running timer
	TA3CTL |= MC_0 + TACLR;
	
	//Timer A2 control
	//TASSEL - source from SMCLK, ID - 1/8 divider, MC - stop timer,
	//TACLR - reset timer to 0, TAIE - enable interrupts, 
	//TAIFG - clear interrupt flag
	TA3CTL = TASSEL_2 + ID_3 + MC_0 + TACLR;
	
	//Timer A2 overflow control
	//enable interrupt on overflow
	TA3CCTL0 = CCIE;
	
	//timer counts up to TA2CCR0 then triggers and resets if in up mode
	//choose value that allows +1 to upper and lower byte without overflow
	TA3CCR0 = 26210;
	
	__delay_cycles(10);
}

void turbine_startTA3(void) {TA3CTL |= MC_1;} //put timer in up mode

void blink(void){
	P7OUT &= ~BIT6;
	wait(50000);
	P7OUT |= BIT6;
	wait(50000);}

void read_my_charfile(char* filen, unsigned char* data1, int num){

	unsigned int br;

	int j;
	rc = f_open(&Fil,filen,FA_READ);
	if (rc) die(rc);
	rc = f_read(&Fil,Buff,num,&br);
	for (j=0; j < num; j++) {data1[j] = Buff[j];}

	rc = f_close(&Fil);
	if (rc) die(rc);
}

void write_my_charfile(const char* filen, unsigned char* data2, int num){

	unsigned int bw;

	rc = f_open(&Fil, filen, FA_WRITE | FA_OPEN_ALWAYS);
	if (rc) die(rc);
	rc = f_lseek(&Fil, f_size(&Fil)); // append data to the end of the file
	if (rc) die(rc);

	int j;
	for (j = 0; j< num; j++)
		Buff[j] = *(data2+j);

	rc = f_write(&Fil, Buff, num, &bw); //512
	if (rc) die(rc);
	rc = f_close(&Fil);
	if (rc) die(rc);
}

void index_file(void){

	unsigned int index;

	rc = f_open(&Fil,"INDEX.txt",FA_READ | FA_OPEN_EXISTING);

	char xname,yname,zname,pname;
	char* suffix = ".txt";
	xname = 'X';
	yname = 'Y';
	zname = 'Z';
	pname = 'P';

	if (rc == FR_NO_FILE) {

		rc = f_close(&Fil); // close file first
		store = '0';

		write_my_charfile("INDEX.txt",&store,1);
	}

	else{
		rc = f_close(&Fil);
		read_my_charfile("INDEX.txt",&store,1);

		index = store -'0';
		if (index < 9){index++;
			store = (char)(((int)'0') + index);

			rc = f_unlink("INDEX.txt");	// In order to overwrite the index file
			write_my_charfile("INDEX.txt",&store,1);}

		else{
			rc = f_unlink("INDEX.txt");
			rc = f_mount(0, &Fatfs);
			//rc = f_unlink();
			//rc = f_mkfs(0,0,0); // Clears everything when index points to 9
			store = '0';
			write_my_charfile("INDEX.txt",&store,1);}	// Reset to initial conditions
	}
	int i = store;	// index of the file

	sprintf(xfile,"%c%c%s",xname,i,suffix);
	sprintf(yfile,"%c%c%s",yname,i,suffix);
	sprintf(zfile,"%c%c%s",zname,i,suffix);
	sprintf(pfile,"%c%c%s",pname,i,suffix);	// construct the names
}

void initTurbine(void){

	setClocks(MCLK_16MHZ, DIVS__2);
	setupBT2(SMCLK, 18, 2);	//setup MSP for pressure spiro
	
	VolToPart[0].pd = 0;
	VolToPart[0].pt = 0;

	P8DIR |= BIT6;
	P8SEL &= ~BIT6;
	P8OUT &= ~BIT6;

    sccb_init(); // setup the disk

	P2REN |= BIT6;  // Enable P2.6 internal resistance
	P2OUT &= ~BIT6;  // Set P2.6 as pull-down resistance
    P2IE |= BIT6;    // P2.6 interrupt enabled
    P2IES &= ~BIT6;  // P2.6 low->high edge
    P2IFG &= ~BIT6;  // P2.6 IFG cleared

    P8OUT |= BIT6;

    P1DIR = P1DIR | TOUCHRESET; // Touch sensor initialization
    P1SEL &= ~TOUCHRESET;
    P1OUT |= TOUCHRESET;

    P1OUT &= ~TOUCHRESET;
    _delay_cycles(10000);
    P1OUT |= TOUCHRESET;

    _delay_cycles(5000000); // Wait for at least 10 ms

    P8SEL &= ~BIT0;
    P8DIR &= ~BIT0;
    P8REN |= BIT0;
    P8OUT |= BIT0;

    P8OUT &= ~BIT6;
    stop();

	int i = init_card(2);
	rc = f_mount(0, &Fatfs);  //initialize and mount the SD card

	index_file();	// Creat new files on the SD card

	temp_counter = 0;
	data_counter = 0;
	buffer_flag = 0;

    I2Cbyte_write(0x3A,0x2A,0x03); // CTRL_REG1: Fast read mode, full-scale range mode active
    I2Cbyte_write(0x3A,0x0E,0x10); // Set measure range to +/- 8g

    __enable_interrupt();

	turbine_setupTA3();
	turbine_startTA3();

	SD24BINCTL0 &=  ~SD24GAIN0 ;    //input control register - SD24BINCTL0
	SD24BINCTL0 &=  ~SD24GAIN1 ;    //A2D Initialization
	SD24BINCTL0 &=  ~SD24GAIN2 ;
	
	SD24BCTL0 = SD24REFS | SD24SSEL_1;        // Select internal REF	// Select SMCLK as SD24_B clock source

	SD24BCCTL0 |= SD24SNGL;                   // Single conversion

	__delay_cycles(0x3600);                   // Delay for 1.5V REF startup

	interrupt_flag = 0;}

void stopTurbine(void){

	TA0CTL |= MC_0 + TACLR; //stop photo-diode 1 timer (Timer A0)
	TA2CTL |= MC_0 + TACLR; //stop photo-diode 2 timer (Timer A1)
}

void ta3ccTurbine(void){

	interrupt_flag = 1;
//	__enable_interrupt();

	xdata = I2Cbyte_read(0x3A,1);	// Read x-axis data
	ydata = I2Cbyte_read(0x3A,3);	// Read y-axis data
	zdata = I2Cbyte_read(0x3A,5);	// Read z-axis data

	//Capturing A2D, the part for pressure sensor...
	SD24BCCTL0 |= SD24SC;                 // Set bit to start conversion
	while ((SD24BIFG & SD24IFG0) == 0) ;  // Poll interrupt flag for channel 2
	pdata = (unsigned char) (SD24BMEMH0 & 0x00FF);

	if (buffer_flag == 0){

	if (temp_counter < 500){

		temp_x[temp_counter] = xdata;
		temp_y[temp_counter] = ydata;
		temp_z[temp_counter] = zdata;
		temp_p[temp_counter] = pdata;

		temp_counter++;}
	}

	else {
		if (data_counter < 10){

		data_x[data_counter] = xdata;
		data_y[data_counter] = ydata;
		data_z[data_counter] = zdata;
		data_p[data_counter] = pdata;

		data_counter++;}
	}

	if (data_counter != 0){
		int c;
		for (c = 0; c < data_counter - 1; c++){
			temp_x[c] = data_x[c];
			temp_y[c] = data_y[c];
			temp_z[c] = data_z[c];
			temp_p[c] = data_p[c];}

		temp_counter = c + 1;
	}

	interrupt_flag = 0;}

void runTurbine(void){

	if (temp_counter == 500){

		buffer_flag = 1;
		temp_counter = 0;

		write_my_charfile(xfile,temp_x,500);
		write_my_charfile(yfile,temp_y,500);
		write_my_charfile(zfile,temp_z,500);
		write_my_charfile(pfile,temp_p,500);

		data_counter = 0;
		buffer_flag = 0;
	}
}

void save_file(void){

	write_my_charfile(xfile,temp_x,temp_counter);
	write_my_charfile(yfile,temp_y,temp_counter);
	write_my_charfile(zfile,temp_z,temp_counter);
	write_my_charfile(pfile,temp_p,temp_counter);
}
